#include "video.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>

namespace emu::bindings
{

int y8_pset(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);
    const auto v = luaL_checkunsigned(state, 3);
    
    if (x >= 128 || y >= 128)
    {
        // Out of bounds
        // TODO: is this the proper behavior?
        return 0;
    }

    emulator.mmio().frame_buffer().set_pixel(x, y, v % 16);

    return 0;
}

int y8_pget(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);

    if (x >= 128 || y >= 128)
    {
        lua_pushunsigned(state, 0);
        return 1;
    }

    lua_pushunsigned(state, emulator.mmio().frame_buffer().get_pixel(x, y));
    return 1;
}

int y8_cls(lua_State* state)
{
    std::uint8_t palette_entry = 0;

    // FIXME: should reset text cursor pos to (0, 0)

    const auto argument_count = lua_gettop(state);

    if (argument_count >= 1)
    {
        palette_entry = luaL_checkunsigned(state, 1);
    }

    emulator.mmio().frame_buffer().clear(palette_entry);

    return 0;
}

}