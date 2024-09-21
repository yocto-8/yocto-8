#include "table.hpp"

#include "lauxlib.h"
#include "ldo.h"
#include "lua.h"

namespace emu::bindings {

int y8_add(lua_State *state) {
	// this roughly matches the definition of table.insert as it appears to
	// match the semantics of `add`

	const int arg_count = lua_gettop(state);

	if (arg_count < 2) {
		// PICO-8 returns no value in that case
		return 0;
	}

	const int table_size = int(lua_rawlen(state, 1) + 1);
	int insert_pos = table_size;

	if (arg_count >= 3) {
		insert_pos = luaL_checkinteger(state, 3);

		// pos must be in [1, e]
		luaL_argcheck(state,
		              (lua_Unsigned)insert_pos - 1u < (lua_Unsigned)table_size,
		              3, "position out of bounds");

		// shift up elements
		for (int i = table_size; i > insert_pos; i--) {
			// t[i] = t[i - 1];
			lua_rawgeti(state, 1, i - 1);
			lua_rawseti(state, 1, i);
		}

		// bring the value up for returning later (otherwise the top of the
		// stack is the index, which is arg #3)
		// we don't need to do this for arg_count == 2 since arg #2 will be the
		// top of the stack already
		lua_pushvalue(state, 2);
	}

	// push the value for the rawseti
	lua_pushvalue(state, 2);

	// t[insert_pos] = v
	lua_rawseti(state, 1, insert_pos);

	return 1;
}

int y8_foreach(lua_State *state) {
	// stack[1]: table
	// stack[2]: function

	// PICO-8 edge cases for foreach:
	// - `foreach()` is a noop (so no checking needed, because #nil will be 0)
	// - `foreach({"foo"})` causes an error (so no checking needed)
	// - `foreach({"foo"}, 1)` causes an error (so no checking needed)

	const auto len = int(lua_rawlen(state, 1));

	for (int i = 1; i <= len;) {
		// TODO: hack something into the VM to be able to avoid the above push
		lua_rawgeti(state, 1, i);

		if (!lua_isnil(state, -1)) {

			lua_pushvalue(state, 2);

			// duplicate value for the equality check after
			lua_pushvalue(state, -2);

			// stack[-3]: value
			// stack[-2]: function
			// stack[-1]: value
			lua_call(state, 1, 0);

			// standard PICO-8 accomodates the deranged usecase of calling del
			// into a table being foreach'd by, apparently, checking whether the
			// item at the current position got modified. we can do the same
			// thing, but let's avoid going through the metatable for equality.
			// if a cart breaks this by using metatables please end my
			// suffering.

			lua_rawgeti(state, 1, i);

			// stack[-1]: old_value
			// stack[-2]: new_value
			if (!lua_rawequal(state, -1, -2)) {
				continue; // without incrementing
			}

		} else {
			// pop the value away
			lua_settop(state, -1);
		}

		++i;
	}

	return 0;
}

} // namespace emu::bindings