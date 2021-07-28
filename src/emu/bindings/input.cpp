#include "input.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>
#include <devices/buttonstate.hpp>

namespace emu::bindings
{

int y8_btn(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    if (argument_count >= 1)
    {
        const auto button = luaL_checkunsigned(state, 1);

        if (argument_count >= 2)
        {
            const auto player = luaL_checkunsigned(state, 2);
            
            // We can only handle one player at the moment
            if (player != 0)
            {
                lua_pushboolean(state, false);
                return 1;
            }
        }

        lua_pushboolean(state, device<devices::ButtonState>.is_pressed(button, 0));
        return 1;
    }

    // Return the entire bitset when no argument is provided
    lua_pushunsigned(state, device<devices::ButtonState>.for_player(0));
    return 1;
}


}