#include "table.hpp"

#include "lauxlib.h"
#include "ldo.h"

namespace emu::bindings {

int y8_add(lua_State *state) {
	// Equivalent to the following Lua code:
	//     local add = function(t, v)
	//         if t == nil then
	//             return nil
	//         end
	//         t[#t+1] = v
	//         return v
	//     end

	const auto table_end_index = luaL_len(state, 1) + 1;
	lua_rawseti(state, 1, table_end_index);

	return 1;
}

int y8_foreach(lua_State *state) {
	// Equivalent to the following Lua code:
	//     local foreach = function(t, f)
	//         for e in all(t) do
	//             f(e)
	//         end
	//     end
	//
	// where all is defined as the following:
	//     if t == nil or #t == 0 then
	//         return function() end
	//     end
	//
	//     local i = 1
	//     local prev = nil
	//
	//     return function()
	//         if t[i] == prev then
	//             i += 1
	//         end
	//
	//         while t[i] == nil and i <= #t do
	//             i += 1
	//         end
	//
	//         prev = t[i]
	//
	//         return prev

	/* table is in the stack at index 't' */
	lua_pushnil(state); // first key
	while (lua_next(state, 1) != 0) {
		// key at -2, value at -1

		// we want to call arg #2 (function) with value at -1
		lua_pushvalue(state, 2); // now value is at -2
		lua_pushvalue(state, -2);
		lua_call(state, 1, 0);

		// we want to remove value and keep key for the next iteration
		lua_pop(state, 1);
	}

	return 0;
}

} // namespace emu::bindings