#pragma once

#include "binding.hpp"

namespace emu::bindings {

int y8_add(lua_State *state);
int y8_del(lua_State *state);
int y8_foreach(lua_State *state);

} // namespace emu::bindings