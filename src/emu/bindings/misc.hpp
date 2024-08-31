#pragma once

#include "binding.hpp"

namespace emu::bindings {

int y8_stat(lua_State *state);
int y8_exit(lua_State *state);
int y8_printh(lua_State *state);
int y8_tostr(lua_State *state);
int y8_sub(lua_State *state);

} // namespace emu::bindings