#include "emulator.hpp"
#include "lgc.h"

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
#include <devices/buttonstate.hpp>
#include <devices/drawpalette.hpp>
#include <devices/screenpalette.hpp>
#include <devices/clippingrectangle.hpp>
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

    device<devices::DrawPalette>.reset();
    device<devices::ScreenPalette>.reset();
    device<devices::ClippingRectangle>.reset();

    bind("pset", bindings::y8_pset);
    bind("pget", bindings::y8_pget);
    bind("cls", bindings::y8_cls);
    bind("rectfill", bindings::y8_rectfill);
    bind("spr", bindings::y8_spr);
    bind("pal", bindings::y8_pal);
    bind("clip", bindings::y8_clip);

    bind("btn", bindings::y8_btn);

    bind("peek", bindings::y8_peek);
    bind("peek2", bindings::y8_peek2);
    bind("peek4", bindings::y8_peek4);
    bind("poke", bindings::y8_poke);
    bind("poke2", bindings::y8_poke2);
    bind("poke4", bindings::y8_poke4);
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
        const auto frame_start_time = hal::measure_time_us();
        auto target_time = 1'000'000u / 60;

        device<devices::ButtonState>.for_player(0) = hal::update_button_state();
        
        // this _update behavior matches pico-8's: if _update60 is defined, _update is ignored
        // when both are unspecified, flipping will occur at 30Hz regardless
        //
        // FIXME: this is not something we really guarantee here atm:
        // we should clip to 30fps if between 30 and 60, to 15 if between 30 and 60
        // also, what about infinite loops (e.g. one drawing stuff constantly?) - does it still flip? (doubt it)

        if (run_hook("_update60") == HookResult::UNDEFINED)
        {
            target_time = 1'000'000u / 30;
            run_hook("_update");
        }

        run_hook("_draw");
        hal::present_frame();

        const auto taken_time = hal::measure_time_us() - frame_start_time;

        if (taken_time < target_time)
        {
            hal::delay_time_us(target_time - taken_time);
        }
    }
}

Emulator::HookResult Emulator::run_hook(const char* name)
{
    lua_getglobal(_lua, name);

    if (!lua_isfunction(_lua, -1))
    {
        lua_pop(_lua, 1);
        return HookResult::UNDEFINED;
    }

    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("hook '%s' execution failed: %s\n", name, lua_tostring(_lua, -1));
        lua_pop(_lua, 1);
        return HookResult::LUA_ERROR;
    }

    return HookResult::SUCCESS;
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

    //printf("malloc stats before alloc/free %d/%d\n", malloc_pool_used, malloc_pool_limit);

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
            //printf("free fast pool %d/%d\n", int(malloc_pool_used), int(malloc_pool_limit));
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
        //printf("alloc fast pool %d/%d\n", int(malloc_pool_used), int(malloc_pool_limit));
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