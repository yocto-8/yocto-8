#pragma once

#include "binding.hpp"

namespace emu::bindings
{

int y8_pset(lua_State* state);
int y8_pget(lua_State* state);
int y8_cls(lua_State* state);
int y8_spr(lua_State* state);

}