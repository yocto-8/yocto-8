#include "rng.hpp"

#include <cstdlib>
#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>

namespace emu::bindings
{

int y8_rnd(lua_State* state)
{
    // FIXME: this does not handle passing a table as a parameter yet
    // FIXME: this does not handle rnd() (which would be rnd(1))
    const auto max = luaL_checknumber(state, 1);

    // FIXME: this does not update random_state() in MMIO, nor does it follow the p8 algorithm.
    lua_pushnumber(state, LuaFix16::from_fix16(rand() % max.value));

    return 1;
}

}