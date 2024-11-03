#include "rng.hpp"

#include <cstdlib>
#include <devices/random.hpp>
#include <emu/emulator.hpp>
#include <lauxlib.hpp>
#include <lua.hpp>

namespace emu::bindings {

int y8_rnd(lua_State *state) {
	// FIXME: this does not handle passing a table as a parameter yet

	const auto argument_count = lua_gettop(state);

	auto range = LuaFix16(1);

	if (argument_count >= 1) {
		range = lua_tonumber(state, 1);
	}

	lua_pushnumber(
		state, LuaFix16::from_fix16(device<devices::Random>.next(range.value)));

	return 1;
}

int y8_srand(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	auto seed = LuaFix16(0);

	if (argument_count >= 1) {
		seed = lua_tonumber(state, 1);
	}

	device<devices::Random>.set_seed(seed.value);

	return 0;
}

} // namespace emu::bindings