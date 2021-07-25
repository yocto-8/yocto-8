#include "mmio.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>

namespace emu::bindings
{

int y8_memcpy(lua_State* state)
{
    const auto dst = luaL_checkunsigned(state, 1);
    const auto src = luaL_checkunsigned(state, 2);
    const auto len = luaL_checkunsigned(state, 3);

    // FIXME: this should properly fix src/dst/len values

    std::memmove(
        emulator.mmio().view.data() + dst,
        emulator.mmio().view.data() + src,
        len
    );

    return 0;
}

}