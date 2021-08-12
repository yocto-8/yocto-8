#include "emulator.hpp"

#include <cstdio>
#include <cstdlib>
#include <limits>
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

void Emulator::init(std::span<std::byte> memory_buffer)
{
    _memory_buffer = memory_buffer;

    _memory.fill(0);

    ta_init(
        _memory_buffer.data(),
        _memory_buffer.data() + _memory_buffer.size(),
        1024, // TODO: what value..?
        16,
        sizeof(std::uintptr_t)
    );

    _lua = lua_newstate(lua_alloc, nullptr);
    luaL_openlibs(_lua);

    device<devices::DrawPalette>.reset();
    device<devices::ScreenPalette>.reset();
    device<devices::ClippingRectangle>.reset();

    const auto bind = [&](const char* name, const auto& func) {
        lua_pushcfunction(_lua, func);
        lua_setglobal(_lua, name);
    };

    const auto stub = [&](const char* name) {
        bind(name, [](lua_State*) {
            //printf("unimplemented blahblah\n");
            return 0;
        });
    };

    stub("camera");
    bind("pset", bindings::y8_pset);
    bind("pget", bindings::y8_pget);
    bind("fget", bindings::y8_fget);
    bind("cls", bindings::y8_cls);
    bind("line", bindings::y8_line);
    stub("circfill");
    bind("rectfill", bindings::y8_rectfill);
    bind("spr", bindings::y8_spr);
    bind("pal", bindings::y8_pal);
    bind("clip", bindings::y8_clip);
    bind("mset", bindings::y8_mset);
    bind("mget", bindings::y8_mget);
    bind("map", bindings::y8_map);

    bind("btn", bindings::y8_btn);

    bind("peek", bindings::y8_peek);
    bind("peek2", bindings::y8_peek2);
    bind("peek4", bindings::y8_peek4);
    bind("poke", bindings::y8_poke);
    bind("poke2", bindings::y8_poke2);
    bind("poke4", bindings::y8_poke4);
    bind("memcpy", bindings::y8_memcpy);
    bind("memset", bindings::y8_memset);

    bind("abs", bindings::y8_abs);
    bind("flr", bindings::y8_flr);
    bind("mid", bindings::y8_mid);
    bind("min", bindings::y8_min);
    bind("max", bindings::y8_max);
    bind("sin", bindings::y8_sin);
    bind("cos", bindings::y8_cos);
    bind("sqrt", bindings::y8_sqrt);
    bind("shl", bindings::y8_shl);
    bind("shr", bindings::y8_shr);
    bind("band", bindings::y8_band);

    bind("rnd", bindings::y8_rnd);

    bind("t", bindings::y8_time);
    bind("time", bindings::y8_time);

    stub("music");
    stub("sfx");

    load(R"(
print("Setting up yocto-8 Lua routines")

function all(t)
    if t == nil or #t == 0 then
        return function() end
    end

    local i = 1
    local prev = nil

    return function()
        if t[i] == prev then
            i += 1
        end

        while t[i] == nil and i <= #t do
            i += 1
        end

        prev = t[i]

        return prev
    end
end

function foreach(t, f)
    for e in all(t) do
        f(e)
    end
end

function add(t, v)
    if t == nil then
        return nil
    end

    t[#t+1] = v
    return v
end

function del(t, v)
    if t == nil then
        return
    end

    local n=#t

    local i
    for i=1,n do
        if t[i] == v then
            for j = i,n-1 do
                t[j] = t[j + 1]
            end

            t[n] = nil
            return v
        end
    end
end

function count(t, v)
    local n = 0
    if v == nil then
        for i = 1,#t do
            if t[i] ~= nil then
                n += 1
            end
        end
    else
        for i = 1,#t do
            if t[i] == v then
                n += 1
            end
        end
    end
    return n
end
)");
}

void Emulator::load(std::string_view buf)
{
    const int load_status = luaL_loadbuffer(_lua, buf.data(), buf.size(), "main");

    if (load_status != 0)
    {
        printf("Script load failed: %s\n", lua_tostring(_lua, -1));
        lua_pop(_lua, 1);
    }

    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("Script exec at load time failed: %s\n", lua_tostring(_lua, -1));
        lua_pop(_lua, 1);
    }
    else
    {
        printf("Loaded segment successfully (%d bytes)\n", int(buf.size()));
    }
}

void Emulator::run()
{
    hal::reset_timer();

    run_hook("_init");
    
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

        printf("%f\n", double(taken_time) / 1000.0);

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

    static std::uint32_t malloc_pool_used = 0;

    static bool can_try_emergency = true;
    static std::uint32_t retry_emergency_when_below = 0;

    // max bytes allocated through malloc. the larger this is, the faster Lua will be, by a lot.
    // note that the real heap usage may be significantly higher, because of:
    // - fragmentation
    // - overhead of the allocator
    const std::size_t malloc_pool_limit = 128 * 1024;

    /*printf("%u;%u;%i;%i;%i\n",
        malloc_pool_used, malloc_pool_limit,
        ta_num_used(), ta_num_free(), ta_num_fresh()
    );*/

    const auto is_slow_heap = [&] {
        // FIXME: this is very naughty
        const auto alloc_buffer = emulator.get_memory_alloc_buffer();
        return ptr >= alloc_buffer.data()
            && ptr < alloc_buffer.data() + alloc_buffer.size();
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

            // if we freed up enough memory, maybe we can try to fit in the malloc pool again
            if (malloc_pool_used <= retry_emergency_when_below)
            {
                can_try_emergency = true;
            }
        }
    };

    const auto auto_malloc = [&]() -> void* {
        std::uint32_t new_malloc_pool_size = malloc_pool_used + nsize;

        if (new_malloc_pool_size > malloc_pool_limit)
        {
            if (can_try_emergency)
            {
                can_try_emergency = false;
                retry_emergency_when_below = malloc_pool_limit - nsize;
                return nullptr;
            }

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

        if (new_ptr == nullptr)
        {
            return nullptr;
        }

        std::memcpy(new_ptr, ptr, osize);
        auto_free();
        return new_ptr;
    }
}

constinit Emulator emulator;

}