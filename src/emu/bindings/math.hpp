#pragma once

#include "binding.hpp"

namespace emu::bindings {

int y8_abs(lua_State *state);
int y8_flr(lua_State *state);
int y8_mid(lua_State *state);
int y8_min(lua_State *state);
int y8_max(lua_State *state);
int y8_sin(lua_State *state);
int y8_cos(lua_State *state);
int y8_sqrt(lua_State *state);
int y8_shl(lua_State *state);
int y8_shr(lua_State *state);
int y8_lshr(lua_State *state);
int y8_rotl(lua_State *state);
int y8_rotr(lua_State *state);
int y8_band(lua_State *state);
int y8_bor(lua_State *state);
int y8_bor(lua_State *state);
int y8_sgn(lua_State *state);

} // namespace emu::bindings