#include "hal/hal.hpp"

#include <coredefs.hpp>
#include <emu/emulator.hpp>
#include <lauxlib.hpp>
#include <lua.hpp>

using namespace y8;

namespace emu::bindings {

int y8_ls(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	lua_createtable(state, 0, 0);

	const char *root_directory = nullptr;

	if (argument_count >= 1) {
		root_directory = lua_tostring(state, 1);
	}

	struct CallbackState {
		lua_State *lua;
		int i;
	};
	CallbackState s{.lua = state, .i = 1};

	hal::fs_list_directory(
		+[](void *ud, const hal::FileInfo &info) {
			auto &state = *static_cast<CallbackState *>(ud);

			lua_pushlstring(state.lua, info.name.data(), info.name.size());

			// stack[-2]: new table
		    // stack[-1]: fname
			lua_rawseti(state.lua, -2, state.i);

			state.i += 1;

			return true;
		},
		&s, root_directory);

	return 1;
}

} // namespace emu::bindings