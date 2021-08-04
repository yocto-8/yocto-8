#pragma once

#include "binding.hpp"

namespace emu::bindings
{

int y8_peek(lua_State* state);
int y8_peek2(lua_State* state);
int y8_peek4(lua_State* state);

int y8_poke(lua_State* state);
int y8_poke2(lua_State* state);
int y8_poke4(lua_State* state);

int y8_memcpy(lua_State* state);
int y8_memset(lua_State* state);

}