#include "emulator.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <cstdio>

namespace emu
{

Emulator::Emulator() :
    _lua(nullptr)
{}

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

    ta_init(
        _memory_buffer.data(),
        _memory_buffer.data() + _memory_buffer.size(),
        1024,
        16,
        sizeof(std::uintptr_t)
    );

    _lua = lua_newstate(lua_alloc, nullptr);
    luaL_openlibs(_lua);

    lua_pushcfunction(_lua, y8_pset);
    lua_setglobal(_lua, "pset");
}

void Emulator::load(gsl::span<const char> buf)
{
    // FIXME: buf.size() causes sad for literal strings because of \0: strip it?
    const int load_status = luaL_loadbuffer(_lua, buf.data(), buf.size(), "main");

    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("Script load failed: %s\n", lua_tostring(_lua, -1));
    }
    else
    {
        printf("Loaded main segment successfully (%d bytes)\n", int(buf.size()));
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
    // FIXME: tonumberx checking
    //luaL_checknumber(state, 1);
    const auto x = lua_tointeger(state, 1);

    //luaL_checknumber(state, 2);
    const auto y = lua_tointeger(state, 2);

    //luaL_checknumber(state, 3);
    const auto v = lua_tointeger(state, 3);

    if (x < 0 || x >= 128 || y < 0 || y >= 128)
    {
        // Out of bounds
        // TODO: is this the proper behavior?
        return 0;
    }

    emulator.frame_buffer.set_pixel(std::uint8_t(x), std::uint8_t(y), std::uint8_t(v % 16));

    return 0;
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
            printf("free fast pool %d/%d\n", malloc_pool_used, malloc_pool_limit);
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
        printf("alloc fast pool %d/%d\n", malloc_pool_used, malloc_pool_limit);
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