#include "emulator.hpp"
#include "devices/drawstatemisc.hpp"
#include "devices/image.hpp"

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <tinyalloc.h>

#include <emu/bindings/input.hpp>
#include <emu/bindings/math.hpp>
#include <emu/bindings/misc.hpp>
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

    bind("camera", bindings::y8_camera);
    bind("pset", bindings::y8_pset);
    bind("pget", bindings::y8_pget);
    bind("sset", bindings::y8_sset);
    bind("sget", bindings::y8_sget);
    bind("fget", bindings::y8_fget);
    bind("cls", bindings::y8_cls);
    bind("line", bindings::y8_line);
    bind("circfill", bindings::y8_circfill);
    bind("rectfill", bindings::y8_rectfill);
    bind("spr", bindings::y8_spr);
    bind("pal", bindings::y8_pal);
    bind("palt", bindings::y8_palt);
    bind("clip", bindings::y8_clip);
    bind("mset", bindings::y8_mset);
    bind("mget", bindings::y8_mget);
    bind("map", bindings::y8_map);
    bind("flip", bindings::y8_flip);
    bind("print", bindings::y8_print);
    bind("_rgbpal", bindings::y8_rgbpal);

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

    bind("printh", bindings::y8_printh);
    bind("sub", bindings::y8_sub);

    bind("rnd", bindings::y8_rnd);

    bind("t", bindings::y8_time);
    bind("time", bindings::y8_time);

    stub("music");
    stub("sfx");

    hal::load_rgb_palette(_palette);

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

function __panic(msg)
    print(":(", 0, 0, 7)
    print(msg)
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

    _frame_target_time = 1'000'000u / get_fps_target();

    run_hook("_init");
    
    for (;;)
    {
        // this _update behavior matches pico-8's: if _update60 is defined, _update is ignored
        // when both are unspecified, flipping will occur at 30Hz regardless
        //
        // FIXME: this is not something we really guarantee here atm:
        // we should clip to 30fps if between 30 and 60, to 15 if between 30 and 60
        // also, what about infinite loops (e.g. one drawing stuff constantly?) - does it still flip? (doubt it)

        if (run_hook("_update60") == HookResult::UNDEFINED)
        {
            run_hook("_update");
        }

        run_hook("_draw");
        
        flip();
    }
}

void Emulator::flip()
{
    hal::present_frame();

    const auto taken_time = hal::measure_time_us() - _frame_start_time;

    lua_gc(_lua, LUA_GCSTEP, 10);

    printf("%f\n", double(taken_time) / 1000.0);

    if (taken_time < _frame_target_time)
    {
        hal::delay_time_us(_frame_target_time - taken_time);
    }
    
    _frame_start_time = hal::measure_time_us();
    
    device<devices::ButtonState>.for_player(0) = hal::update_button_state();
}

int Emulator::get_fps_target() const
{
    lua_getglobal(_lua, "_update60");
    const bool is60 = lua_isfunction(_lua, -1);
    lua_pop(_lua, 1);

    return is60 ? 60 : 30;
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
        panic(lua_tostring(_lua, -1));
        lua_pop(_lua, 1);
        //return HookResult::LUA_ERROR;
    }

    return HookResult::SUCCESS;
}

void Emulator::panic(const char* message)
{
    device<devices::DrawPalette>.reset();
    device<devices::ScreenPalette>.reset();
    //device<devices::DrawStateMisc>.reset();
    device<devices::Framebuffer>.clear(0);

    lua_getglobal(_lua, "__panic");
    lua_pushstring(_lua, message);

    lua_pcall(_lua, 1, 0, 0);

    hal::present_frame();

    for (;;);
}

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
    (void)ud;

    // This is a Lua allocation function that uses standard malloc and realloc,
    // but can make use of a secondary memory pool as a fallback using tinyalloc.

    // The secondary memory pool is used only when malloc() fails to allocate.
    // For performance reasons, it is a good idea to aggressively GC so that heap
    // usage remains as low as possible since the secondary memory pool may be
    // significantly slower.

    // We used to (ab)use the Lua emergency GC by strategically returning nullptr
    // to force GC to occur when the primary pool is exhausted, but this caused
    // spikes and using LUA_GCSTEP regularly enough appears to mitigate the
    // problem rather well.

    // The idea of letting Lua consume all of the malloc() heap is somewhat fine
    // for us, because we never allocate memory dynamically elsewhere.
    // (TODO: is that really true, though? can newlib printf malloc?)

    const auto is_slow_heap = [&] {
        const auto alloc_buffer = emulator.get_memory_alloc_buffer();
        return ptr >= alloc_buffer.data()
            && ptr < alloc_buffer.data() + alloc_buffer.size();
    };

    // Free this pointer no matter the heap it was allocated in.
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
        }
    };

    // 
    const auto auto_malloc = [&]() -> void* {
        void* malloc_ptr = malloc(nsize);

        if (malloc_ptr != nullptr)
        {
            return malloc_ptr;
        }

        return ta_alloc(nsize);
    };

    const auto slow_realloc = [&]() -> void* {
        const auto new_ptr = auto_malloc();

        if (new_ptr == nullptr)
        {
            return nullptr;
        }

        std::memcpy(new_ptr, ptr, osize);
        auto_free();
        return new_ptr;
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
    else if (nsize == osize)
    {
        // this does happen and i don't know if this is handled quickly by newlib realloc()
        return ptr;
    }
    else if (nsize < osize)
    {
        if (!is_slow_heap())
        {
            // here we assume this never fails: can it really not?
            return realloc(ptr, nsize);
        }
        
        return slow_realloc();
    }
    else
    {
        return slow_realloc();
    }
}

constinit Emulator emulator;

}