#include "emulator.hpp"
#include "devices/drawstatemisc.hpp"
#include "devices/image.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <frozen/set.h>

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

    if (!memory_buffer.empty())
    {
        /*umm_init_heap(
            _memory_buffer.data(),
            _memory_buffer.size_bytes());*/
    }
    
    const auto default_palette = hal::get_default_palette();
    std::copy(default_palette.begin(), default_palette.end(), _palette.begin());

    _lua = lua_newstate(lua_alloc, nullptr);
    luaL_openlibs(_lua);

    device<devices::DrawPalette>.reset();
    device<devices::DrawStateMisc>.reset();
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

    // TODO: make lua use this directly; and define a const string table
    using Binding = int(lua_State*);
    constexpr frozen::unordered_map<frozen::string, Binding*, 52> y8_std = {
        {"camera", bindings::y8_camera},
        {"color", bindings::y8_color},
        {"pset", bindings::y8_pset},
        {"pget", bindings::y8_pget},
        {"sset", bindings::y8_sset},
        {"sget", bindings::y8_sget},
        {"fget", bindings::y8_fget},
        {"cls", bindings::y8_cls},
        {"line", bindings::y8_line},
        {"circfill", bindings::y8_circfill},
        {"rectfill", bindings::y8_rectfill},
        {"spr", bindings::y8_spr},
        {"sspr", bindings::y8_sspr},
        {"pal", bindings::y8_pal},
        {"palt", bindings::y8_palt},
        {"fillp", bindings::y8_fillp},
        {"clip", bindings::y8_clip},
        {"mset", bindings::y8_mset},
        {"mget", bindings::y8_mget},
        {"map", bindings::y8_map},
        {"flip", bindings::y8_flip},
        {"print", bindings::y8_print},
        {"_rgbpal", bindings::y8_rgbpal},

        {"btn", bindings::y8_btn},

        {"peek", bindings::y8_peek},
        {"peek2", bindings::y8_peek2},
        {"peek4", bindings::y8_peek4},
        {"poke", bindings::y8_poke},
        {"poke2", bindings::y8_poke2},
        {"poke4", bindings::y8_poke4},
        {"memcpy", bindings::y8_memcpy},
        {"memset", bindings::y8_memset},

        {"abs", bindings::y8_abs},
        {"flr", bindings::y8_flr},
        {"mid", bindings::y8_mid},
        {"min", bindings::y8_min},
        {"max", bindings::y8_max},
        {"sin", bindings::y8_sin},
        {"cos", bindings::y8_cos},
        {"sqrt", bindings::y8_sqrt},
        {"shl", bindings::y8_shl},
        {"shr", bindings::y8_shr},
        {"band", bindings::y8_band},
        {"bor", bindings::y8_bor},

        {"cursor", bindings::y8_cursor},
        {"printh", bindings::y8_printh},
        {"stat", bindings::y8_stat},
        {"sub", bindings::y8_sub}, // OwO
        {"_exit", bindings::y8_exit},

        {"rnd", bindings::y8_rnd},

        {"t", bindings::y8_time},
        {"time", bindings::y8_time},
    };

    for (const auto& [func, binding] : y8_std)
    {
        lua_pushcfunction(_lua, binding);
        lua_setglobal(_lua, func.data());
    }

    stub("music");
    stub("sfx");

    hal::load_rgb_palette(_palette);

    load(R"(
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
    printh("PANIC: " .. msg)
    print(":(", 0, 0, 7)
    print(msg)
end

printh(stat(0) .. "KB at boot")
)");
}

void Emulator::load(std::string_view buf)
{
    const int load_status = luaL_loadbuffer(_lua, buf.data(), buf.size(), "main");

    if (load_status != 0)
    {
        printf("Script load failed: %s\n", lua_tostring(_lua, -1));
        panic(lua_tostring(_lua, -1));
        lua_pop(_lua, 1);
    }

    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("Script exec at load time failed: %s\n", lua_tostring(_lua, -1));
        panic(lua_tostring(_lua, -1));
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

        _update_start_time = hal::measure_time_us();

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

    //printf("%f\n", double(taken_time) / 1000.0);

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

    exit(1);
}

#include "tinyalloc.hpp"
#include <pico/platform.h>

[[gnu::flatten]]
void* __not_in_flash_func(lua_alloc)(void* ud, void* ptr, size_t osize, size_t nsize)
{
    (void)ud;

    static bool fuck = false;
    if (!fuck)
    {
        ta_init();
        fuck = true;
    }

    // This is a Lua allocation function that uses standard malloc and realloc,
    // but can make use of a secondary memory pool as a fallback using umm_malloc.

    // The secondary memory pool is used only when malloc() fails to allocate.
    // For performance reasons, it is a good idea to aggressively GC so that heap
    // usage remains as low as possible since the secondary memory pool may be
    // significantly slower.

    // We (ab)use the Lua emergency GC by strategically returning nullptr to force
    // the GC to kick in when the primary pool is exhausted.
    // In the emulator we try to make the GC step a bit, but while it helps reduce
    // GC-induced spikes it is not a perfect solution and is only performed once
    // per frame, whereas the program may be trying to allocate and free a lot within
    // one frame.

    // The idea of letting Lua consume all of the malloc() heap is somewhat fine
    // for us, because we never allocate memory dynamically elsewhere.

    static bool has_alloc_succeeded_since_egc = false;

    // unfortunately using UMM comes with alignment issues on 64-bit platforms;
    // let's disable the extra heap on these for now.
    const bool has_extra_heap =
        !emulator.get_memory_alloc_buffer().empty()
        && sizeof(void*) <= 4;

    const auto is_ptr_on_slow_heap = [&] {
        const auto alloc_buffer = emulator.get_memory_alloc_buffer();
        return has_extra_heap
            && ptr >= alloc_buffer.data()
            && ptr < alloc_buffer.data() + alloc_buffer.size();
    };

    // Free this pointer no matter the heap it was allocated in.
    const auto auto_free = [&] {
        if (ptr == nullptr)
        {
            return;
        }

        if (is_ptr_on_slow_heap())
        {
            ta_free(ptr);
        }
        else
        {
            free(ptr);
        }
    };

    const auto auto_malloc = [&]() -> void* {
        void* malloc_ptr = malloc(nsize);

        if (malloc_ptr != nullptr)
        {
            /*if (!has_alloc_succeeded_since_egc)
            {
                printf("recovered enough mem\n");
            }*/
            has_alloc_succeeded_since_egc = true;
            return malloc_ptr;
        }

        if (has_alloc_succeeded_since_egc)
        {
            // trigger Lua's EGC
            has_alloc_succeeded_since_egc = false;
            //printf("EGC\n");
            return nullptr;
        }

        if (!has_extra_heap)
        {
            return nullptr;
        }

        return ta_alloc(nsize);
    };

    const auto realloc_from_main_to_extra_heap = [&]() -> void* {
        if (!has_extra_heap)
        {
            return nullptr;
        }

        const auto new_ptr = ta_alloc(nsize);

        if (new_ptr == nullptr)
        {
            printf("slow heap exhausted!!\n");
            return nullptr;
        }

        std::memcpy(new_ptr, ptr, std::min(osize, nsize));
        free(ptr);
        return new_ptr;
    };
    
    const auto slow_realloc = [&]() -> void* {
        const auto new_ptr = auto_malloc();

        if (new_ptr == nullptr)
        {
            return nullptr;
        }

        std::memcpy(new_ptr, ptr, std::min(osize, nsize));
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
        // this does happen; so let's not bother accidentally figuring out
        // if we're triggering some bad behavior un umm_realloc/newlib realloc
        return ptr;
    }
    else if (nsize < osize)
    {
        if (!is_ptr_on_slow_heap())
        {
            const auto nptr = realloc(ptr, nsize);

            if (nptr != nullptr)
            {
                return nptr;
            }

            return realloc_from_main_to_extra_heap();
        }
        
        return slow_realloc();
    }
    else // nsize > osize
    {
        return slow_realloc();
    }
}

constinit Emulator emulator;

}