#include "hal/hal.hpp"
#include "input.hpp"

#include <cstdlib>
#include <emu/emulator.hpp>
#include <lua.h>
#include <lauxlib.h>

namespace emu::bindings
{

enum class StatEntry : std::uint16_t
{
    RAM_USAGE_KB = 0,
    CPU_USAGE_SINCE_UPDATE = 1,
    SYSTEM_CPU_USAGE = 2,
    MAP_DISPLAY = 3,
    CLIPBOARD_STRING = 4,
    P8_VERSION = 5,
    LOAD_PARAMETER = 6,
    CURRENT_FPS = 7,
    TARGET_FPS = 8,
    SYSTEM_FPS = 9,
    // 10 is unknown/reserved
    ENABLED_DISPLAY_COUNT = 11,
    PAUSE_MENU_UL_X = 12,
    PAUSE_MENU_UL_Y = 13,
    PAUSE_MENU_BR_X = 14,
    PAUSE_MENU_BR_Y = 15,
    // TODO: deprecated audio stuff 16..26
    // 27 is unknown/reserved
    RAW_KEYBOARD = 28, // with 2nd scancode param
    // 29 is unknown
    // TODO: devkit mouse/keyboard entries/stubs 30..39
    // TODO: music/sound 46..56
    // 57..71 is unknown, but we could match nil/empty str behavior
    // TODO: time of day 80..95
    PRE_GC_RAM_USAGE_KB = 99,
    // TODO: 100..102 BBS
    // 103..107 is unknown
    // TODO: 110 is "frame-by-frame mode flag"
    // 111..119 is unknown
    // TODO: 120..121 GPIO functionality thingie
    // 122..?? is unknown
};

int y8_stat(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto entry = lua_tointeger(state, 1);

    // FIXME: this is a stub implementation.
    // some games use this for meaningful things, and some of the entries return strings.

    switch (StatEntry(entry))
    {
        case StatEntry::RAM_USAGE_KB:
        {
            // p8 allegedly GCs before stat(0)
            lua_gc(state, LUA_GCCOLLECT, 0);

            const auto usage_kb_part = lua_gc(state, LUA_GCCOUNT, 0);
            const auto usage_b_part = lua_gc(state, LUA_GCCOUNTB, 0);
            lua_pushnumber(state, LuaFix16(std::int16_t(usage_kb_part), usage_b_part));
            break;
        }

        case StatEntry::CPU_USAGE_SINCE_UPDATE:
        {
            const auto current_time = hal::measure_time_us();

            // let's compute this with floats -- not sure if fixed point would overflow
            const auto delta_time = float(current_time - emu::emulator.get_update_start_time());
            const auto budget_spent = delta_time / float(emu::emulator.get_frame_target_time());

            lua_pushnumber(state, LuaFix16(budget_spent));

            break;
        }

        default:
        {
            // unhandled; return 0 by default
            lua_pushnumber(state, LuaFix16());
            return 1;
        }
    }

    return 1;
}

int y8_exit(lua_State* state)
{
    std::exit(lua_tounsigned(state, 1));
}

int y8_printh(lua_State* state)
{
    // based on lua standard print()
    // FIXME: i don't think standard printh in pico-8 uses tostring.

    int n = lua_gettop(state);  /* number of arguments */
    int i;
    lua_getglobal(state, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        size_t l;
        lua_pushvalue(state, -1);  /* function to be called */
        lua_pushvalue(state, i);   /* value to print */
        lua_call(state, 1, 1);
        s = lua_tolstring(state, -1, &l);  /* get result */
        if (s == nullptr)
        {
            return luaL_error(state,
            LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        }
        if (i>1)
        {
            printf("\t");
        }
        
        printf("%s", s);
        lua_pop(state, 1);  /* pop result */
    }
    printf("\n");
    return 0;
}

// based on lua standard sub()
static size_t posrelat(ptrdiff_t pos, size_t len) {
    if (pos >= 0) return (size_t)pos;
    else if (0u - (size_t)pos > len) return 0;
    else return len - ((size_t)-pos) + 1;
}

int y8_sub(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t start = posrelat(luaL_checkinteger(L, 2), l);
    size_t end = posrelat(luaL_optinteger(L, 3, -1), l);
    if (start < 1) start = 1;
    if (end > l) end = l;
    if (start <= end)
        lua_pushlstring(L, s + start - 1, end - start + 1);
    else lua_pushliteral(L, "");
    return 1;
}

}