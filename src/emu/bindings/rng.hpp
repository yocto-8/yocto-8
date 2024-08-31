#pragma once

#include "binding.hpp"

namespace emu::bindings {

int y8_rnd(lua_State *state);
int y8_srand(lua_State *state);

} // namespace emu::bindings