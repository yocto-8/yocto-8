#include "math.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>

namespace emu::bindings
{

int y8_flr(lua_State* state)
{
    const auto x = luaL_checknumber(state, 1);
    lua_pushnumber(state, floor(x));
    return 1;
}

int y8_cos(lua_State* state)
{
    const auto x = luaL_checknumber(state, 1);
    lua_pushnumber(state, (x * (LuaFix16::from_fix16(fix16_pi) * 2)).cos());
    return 1;
}

int y8_sqrt(lua_State* state)
{
    const auto x = luaL_checknumber(state, 1);
    lua_pushnumber(state, x.sqrt());
    return 1;
}

}