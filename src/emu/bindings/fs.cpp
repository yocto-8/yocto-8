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

int y8_cd(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	if (argument_count >= 1) {
		std::string_view path;
		{ // read input into a string_view
			std::size_t path_size;
			const char *path_buf = lua_tolstring(state, 1, &path_size);
			path = {path_buf, path_size};
		}

		hal::fs_set_working_directory(path);
	}

	return 0;
}

} // namespace emu::bindings