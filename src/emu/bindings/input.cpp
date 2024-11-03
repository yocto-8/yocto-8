#include "input.hpp"

#include <devices/buttonstate.hpp>
#include <emu/emulator.hpp>
#include <lauxlib.hpp>
#include <lua.hpp>

namespace emu::bindings {

int y8_btn(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const auto mask = emu::emulator.get_button_state().held_key_mask;

	if (argument_count >= 1) {
		const auto button = lua_tounsigned(state, 1);

		if (argument_count >= 2) {
			const auto player = lua_tounsigned(state, 2);

			// We can only handle one player at the moment
			if (player != 0) {
				lua_pushboolean(state, false);
				return 1;
			}
		}

		lua_pushboolean(state, (mask >> button) & 0b1);
		return 1;
	}

	// Return the entire bitset when no argument is provided
	lua_pushunsigned(state, mask);
	return 1;
}

int y8_btnp(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const auto mask = emu::emulator.get_button_state().pressed_key_mask;

	if (argument_count >= 1) {
		const auto button = lua_tounsigned(state, 1);

		if (argument_count >= 2) {
			const auto player = lua_tounsigned(state, 2);

			// We can only handle one player at the moment
			if (player != 0) {
				lua_pushboolean(state, false);
				return 1;
			}
		}

		lua_pushboolean(state, (mask >> button) & 0b1);
		return 1;
	}

	// Return the entire bitset when no argument is provided
	lua_pushunsigned(state, mask);
	return 1;
}

} // namespace emu::bindings