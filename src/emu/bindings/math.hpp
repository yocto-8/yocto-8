#pragma once

#include "binding.hpp"

namespace emu::bindings
{

int y8_flr(lua_State* state);
int y8_mid(lua_State* state);
int y8_sin(lua_State* state);
int y8_cos(lua_State* state);
int y8_sqrt(lua_State* state);
int y8_shl(lua_State* state);
int y8_shr(lua_State* state);
int y8_band(lua_State* state);

}