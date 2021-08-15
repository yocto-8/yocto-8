#include "mmio.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>

namespace emu::bindings
{

int y8_peek(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto address = luaL_checkunsigned(state, 1);

    std::size_t n = 1;
    if (argument_count >= 2)
    {
        n = std::min(luaL_checkunsigned(state, 2), 8192u);
    }

    for (std::size_t i = 0; i < n; ++i)
    {
        lua_pushunsigned(state, emu::emulator.memory().get<std::uint8_t>(address + i));
    }

    return n;
}

int y8_peek2(lua_State* state)
{
    const auto address = luaL_checkunsigned(state, 1);
    lua_pushunsigned(state, emu::emulator.memory().get<std::uint16_t>(address));
    return 1;
}

int y8_peek4(lua_State* state)
{
    const auto address = luaL_checkunsigned(state, 1);
    lua_pushunsigned(state, emu::emulator.memory().get<std::uint32_t>(address));
    return 1;
}

int y8_poke(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto address = luaL_checkunsigned(state, 1);

    for (int i = 0; i < argument_count - 1; ++i)
    {
        // first parameter is arg 1, first value is arg 2
        const auto value = luaL_checkunsigned(state, i + 2);
        emu::emulator.memory().get<std::uint8_t>(address + i) = value;
    }

    return 0;
}

int y8_poke2(lua_State* state)
{
    const auto address = luaL_checkunsigned(state, 1);
    const auto value = luaL_checkunsigned(state, 2);
    emu::emulator.memory().get<std::uint16_t>(address) = value;
    return 0;
}

int y8_poke4(lua_State* state)
{
    const auto address = luaL_checkunsigned(state, 1);
    const auto value = luaL_checkunsigned(state, 2);
    emu::emulator.memory().get<std::uint32_t>(address) = value;
    return 0;
}

int y8_memcpy(lua_State* state)
{
    const auto dst = luaL_checkunsigned(state, 1);
    const auto src = luaL_checkunsigned(state, 2);
    const auto len = luaL_checkunsigned(state, 3);

    // FIXME: this should properly fix src/dst/len values
    // should this or Memory::memcpy perform bounds checking?
    emulator.memory().memcpy(dst, src, len);

    return 0;
}

int y8_memset(lua_State* state)
{
    const auto dst = luaL_checkunsigned(state, 1);
    const std::uint8_t val = luaL_checkunsigned(state, 2);
    const auto len = luaL_checkunsigned(state, 3);

    emulator.memory().memset(dst, val, len);

    return 0;
}

}