#include "table.hpp"

#include "lauxlib.h"

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

} // namespace emu::bindings