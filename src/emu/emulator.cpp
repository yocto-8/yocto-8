#include "emulator.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <cstdio>
#include <hal/hal.hpp>

namespace emu
{

Emulator::~Emulator()
{
    if (_lua != nullptr)
    {
        lua_close(_lua);
    }
}

void Emulator::init(gsl::span<char> memory_buffer)
{
    _memory_buffer = memory_buffer;

    _memory.fill(0);

    ta_init(
        _memory_buffer.data(),
        _memory_buffer.data() + _memory_buffer.size(),
        1024,
        16,
        sizeof(std::uintptr_t)
    );

    _lua = lua_newstate(lua_alloc, nullptr);
    luaL_openlibs(_lua);

    const auto bind = [&](const char* name, const auto& func) {
        lua_pushcfunction(_lua, func);
        lua_setglobal(_lua, name);
    };

    bind("pset", y8_pset);
    bind("pget", y8_pget);
    bind("cls", y8_cls);

    bind("btn", y8_btn);

    bind("memcpy", y8_memcpy);

    bind("flr", y8_flr);

    bind("rnd", y8_rnd);
}

void Emulator::load(gsl::span<const char> buf)
{
    // FIXME: buf.size() causes sad for literal strings because of \0: strip it?
    const int load_status = luaL_loadbuffer(_lua, buf.data(), buf.size(), "main");

    if (load_status != 0)
    {
        printf("Script load failed: %s", lua_tostring(_lua, -1));
    }

    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("Script exec at load time failed: %s\n", lua_tostring(_lua, -1));
    }
    else
    {
        printf("Loaded main segment successfully (%d bytes)\n", int(buf.size()));
    }
}

void Emulator::run()
{
    for (;;)
    {
        _button_state = hal::update_button_state();
        hook_update();
        hal::present_frame(frame_buffer());
    }
}

void Emulator::hook_update()
{
    lua_getglobal(_lua, "_update");
    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("_update failed: %s\n", lua_tostring(_lua, -1));
    }
}

int Emulator::y8_pset(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);
    const auto v = luaL_checkunsigned(state, 3);
    
    if (x >= 128 || y >= 128)
    {
        // Out of bounds
        // TODO: is this the proper behavior?
        return 0;
    }

    emulator.frame_buffer().set_pixel(x, y, v % 16);

    return 0;
}

int Emulator::y8_pget(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);

    if (x >= 128 || y >= 128)
    {
        lua_pushunsigned(state, 0);
        return 1;
    }

    lua_pushunsigned(state, emulator.frame_buffer().get_pixel(x, y));
    return 1;
}

int Emulator::y8_cls(lua_State* state)
{
    std::uint8_t palette_entry = 0;

    // FIXME: should reset text cursor pos to (0, 0)

    const auto argument_count = lua_gettop(state);

    if (argument_count >= 1)
    {
        palette_entry = luaL_checkunsigned(state, 1);
    }

    emulator.frame_buffer().clear(palette_entry);

    return 0;
}

int Emulator::y8_btn(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    if (argument_count >= 1)
    {
        const auto button = luaL_checkunsigned(state, 1);

        if (argument_count >= 2)
        {
            const auto player = luaL_checkunsigned(state, 2);
            
            // We can only handle one player at the moment
            if (player != 0)
            {
                lua_pushboolean(state, false);
                return 1;
            }
        }

        // Return whether the nth button is pressed as a boolean
        lua_pushboolean(state, ((emulator._button_state >> button) & 0b1) != 0);
        return 1;
    }

    // Return the entire bitset when no argument is provided
    lua_pushunsigned(state, emulator._button_state);
    return 1;
}

int Emulator::y8_memcpy(lua_State* state)
{
    const auto dst = luaL_checkunsigned(state, 1);
    const auto src = luaL_checkunsigned(state, 2);
    const auto len = luaL_checkunsigned(state, 3);

    // FIXME: this should properly fix src/dst/len values

    std::memmove(
        emulator._memory.data() + dst,
        emulator._memory.data() + src,
        len
    );

    return 0;
}

int Emulator::y8_flr(lua_State* state)
{
    const auto x = luaL_checknumber(state, 1);
    lua_pushnumber(state, floor(x));
    return 1;
}

int Emulator::y8_rnd(lua_State* state)
{
    // FIXME: this does not handle passing a table as a parameter yet
    const auto max = luaL_checknumber(state, 1);

    // FIXME: this does not update random_state() in MMIO, nor does it follow the p8 algorithm.
    lua_pushnumber(state, LuaFix16::from_fix16(rand() % max.value));

    return 1;
}

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
    (void)ud;

    //printf("blk usage %d used/%d free/%d fresh\n", ta_num_used(), ta_num_free(), ta_num_fresh());

    static std::uint32_t malloc_pool_used = 0;

    // max bytes allocated through malloc. the larger this is, the faster Lua will be, by a lot.
    // note that the real heap usage may be significantly higher, because of:
    // - fragmentation
    // - overhead of the allocator
    const std::size_t malloc_pool_limit = 96 * 1024;

    const auto is_slow_heap = [&] {
        // FIXME: this is very naughty
        return std::uintptr_t(ptr) >= 0x2F000000;
    };

    const auto auto_free = [&] {
        if (ptr == nullptr)
        {
            return;
        }

        if (is_slow_heap())
        {
            ta_free(ptr);
        }
        else
        {
            free(ptr);
            malloc_pool_used -= osize;
            printf("free fast pool %d/%d\n", int(malloc_pool_used), int(malloc_pool_limit));
        }
    };

    const auto auto_malloc = [&] {
        std::uint32_t new_malloc_pool_size = malloc_pool_used + nsize;

        if (new_malloc_pool_size > malloc_pool_limit)
        {
            // allocate on slow pool
            return ta_alloc(nsize);
        }
        
        // we've got spare space in the malloc pool, allocate there
        malloc_pool_used = new_malloc_pool_size;
        printf("alloc fast pool %d/%d\n", int(malloc_pool_used), int(malloc_pool_limit));
        return malloc(nsize);
    };


    if (nsize == 0)
    {
        auto_free();
        return nullptr;
    }
    else if (ptr == nullptr)
    {
        return auto_malloc();
    }
    else if (nsize <= osize)
    {
        // whatever
        return ptr;
    }
    else
    {
        const auto new_ptr = auto_malloc();
        std::memcpy(new_ptr, ptr, osize);
        auto_free();
        return new_ptr;
    }
}

Emulator emulator;

}