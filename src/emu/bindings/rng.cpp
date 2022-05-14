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

    const auto argument_count = lua_gettop(state);

    auto range = LuaFix16(1);

    if (argument_count >= 1)
    {
        range = lua_tonumber(state, 1);
    }

    // FIXME: this does not update random_state() in MMIO, nor does it follow the p8 algorithm.
    lua_pushnumber(state, LuaFix16::from_fix16(rand() % range.value));

    return 1;
}

}