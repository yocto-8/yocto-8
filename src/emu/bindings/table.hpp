#pragma once

#include "binding.hpp"

namespace emu::bindings {

Y8_FAST_BINDING int y8_add(lua_State *state);
int y8_del(lua_State *state);
Y8_FAST_BINDING int y8_foreach(lua_State *state);
int y8_split(lua_State *state);
int y8_unpack(lua_State *state);

} // namespace emu::bindings