#include "emulator.hpp"

#include <cstdio>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <tinyalloc.h>

#include <emu/bindings/input.hpp>
#include <emu/bindings/math.hpp>
#include <emu/bindings/mmio.hpp>
#include <emu/bindings/video.hpp>
#include <emu/bindings/rng.hpp>
#include <emu/bindings/time.hpp>
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

    bind("pset", bindings::y8_pset);
    bind("pget", bindings::y8_pget);
    bind("cls", bindings::y8_cls);

    bind("btn", bindings::y8_btn);

    bind("memcpy", bindings::y8_memcpy);

    bind("flr", bindings::y8_flr);
    bind("cos", bindings::y8_cos);
    bind("sqrt", bindings::y8_sqrt);

    bind("rnd", bindings::y8_rnd);

    bind("t", bindings::y8_time);
    bind("time", bindings::y8_time);
}

void Emulator::load(std::string_view buf)
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
    hal::reset_timer();
    
    for (;;)
    {
        mmio().button_state() = hal::update_button_state();
        hook_update();
        hal::present_frame();
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