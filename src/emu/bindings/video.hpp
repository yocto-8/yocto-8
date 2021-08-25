#pragma once

#include "binding.hpp"

namespace emu::bindings
{

int y8_camera(lua_State* state);
int y8_pset(lua_State* state);
int y8_pget(lua_State* state);
int y8_sset(lua_State* state);
int y8_sget(lua_State* state);
int y8_fget(lua_State* state);
int y8_cls(lua_State* state);
int y8_line(lua_State* state);
int y8_circfill(lua_State* state);
int y8_rectfill(lua_State* state);
int y8_spr(lua_State* state);
int y8_pal(lua_State* state);
int y8_palt(lua_State* state);
int y8_clip(lua_State* state);
int y8_mset(lua_State* state);
int y8_mget(lua_State* state);
int y8_map(lua_State* state);
int y8_flip(lua_State* state);
int y8_print(lua_State* state);
int y8_rgbpal(lua_State* state); // y8 extension

}