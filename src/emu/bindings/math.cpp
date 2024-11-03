#include "math.hpp"

#include <emu/emulator.hpp>
#include <lauxlib.hpp>
#include <lua.hpp>

namespace emu::bindings {

int y8_abs(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	lua_pushnumber(state, LuaFix16::from_fix16(fix16_abs(x.value)));
	return 1;
}

int y8_flr(lua_State *state) {
	const auto x = luaL_checknumber(state, 1);
	lua_pushnumber(state, floor(x));
	return 1;
}

int y8_mid(lua_State *state) {
	const auto a = lua_tonumber(state, 1);
	const auto b = lua_tonumber(state, 2);
	const auto c = lua_tonumber(state, 3);

	lua_pushnumber(state,
	               std::max(std::min(a, b), std::min(std::max(a, b), c)));

	return 1;
}

int y8_min(lua_State *state) {
	const auto a = lua_tonumber(state, 1);
	const auto b = lua_tonumber(state, 2);

	lua_pushnumber(state, std::min(a, b));

	return 1;
}

int y8_max(lua_State *state) {
	const auto a = lua_tonumber(state, 1);
	const auto b = lua_tonumber(state, 2);

	lua_pushnumber(state, std::max(a, b));

	return 1;
}

int y8_sin(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	lua_pushnumber(state, -(x * (LuaFix16::from_fix16(TWO_PI))).sin());
	return 1;
}

int y8_cos(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	lua_pushnumber(state, (x * (LuaFix16::from_fix16(TWO_PI))).cos());
	return 1;
}

int y8_atan2(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto y = lua_tonumber(state, 2);
	if (x == 0 && y == 0) {
		lua_pushnumber(state, LuaFix16(0.25));
	} else {
		lua_pushnumber(state,
		               (y.atan2(x) / -LuaFix16::from_fix16(TWO_PI) + 1) % 1);
	}
	return 1;
}

int y8_sqrt(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	lua_pushnumber(state, x.sqrt());
	return 1;
}

int y8_shl(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto bits = lua_tonumber(state, 2);
	lua_pushnumber(state, x << bits);
	return 1;
}

int y8_shr(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto bits = lua_tonumber(state, 2);
	lua_pushnumber(state, x >> bits);
	return 1;
}

int y8_lshr(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto bits = lua_tonumber(state, 2);
	lua_pushnumber(state, x.unsigned_right_shift(bits));
	return 1;
}

int y8_rotl(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto bits = lua_tonumber(state, 2);
	lua_pushnumber(state, x.rotate_left(bits));
	return 1;
}

int y8_rotr(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto bits = lua_tonumber(state, 2);
	lua_pushnumber(state, x.rotate_right(bits));
	return 1;
}

int y8_band(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto y = lua_tonumber(state, 2);
	lua_pushnumber(state, LuaFix16::from_fix16(x.value & y.value));
	return 1;
}

int y8_bor(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	const auto y = lua_tonumber(state, 2);
	lua_pushnumber(state, LuaFix16::from_fix16(x.value | y.value));
	return 1;
}

int y8_sgn(lua_State *state) {
	const auto x = lua_tonumber(state, 1);
	lua_pushinteger(state, x >= 0 ? 1 : -1);
	return 1;
}

} // namespace emu::bindings