#include "table.hpp"

#include "lapi.h"
#include "lauxlib.h"
#include "lobject.h"
#include "ltable.h"
#include "lua.h"
#include "lvm.h"
#include <string_view>

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

int y8_del(lua_State *state) {
	// stack[1]: table
	// stack[2]: value

	const auto arg_count = lua_gettop(state);

	if (arg_count < 2) {
		// PICO-8 returns no value in that case
		return 0;
	}

	// NOTE: del only cares about sequences. we do not actually need to iterate
	// over the hashmap part of the table

	const auto len = int(lua_rawlen(state, 1));

	for (int i = 1; i <= len; ++i) {
		// TODO: index access here ignores the metatable; do we care?
		lua_rawgeti(state, 1, i);

		// honor metatable
		if (equalobj(state, index2addr(state, 2), index2addr(state, -1))) {
			// found elem to remove
			// note that by this point, the value is still in the stack (we do
			// want to return it)

			// shift down the end of the array one to the left

			for (int j = i; j < len; ++j) {
				// t[j] = t[j + 1];
				lua_rawgeti(state, 1, j + 1);
				lua_rawseti(state, 1, j);
			}

			// finally, set the last elem to nil
			lua_pushnil(state);
			lua_rawseti(state, 1, len);

			// return the value
			return 1;
		} else {
			// remove value from stack, we're done inspecting it
			lua_settop(state, -2);
		}
	}

	// PICO-8 returns no value (not nil) when nothing was removed
	// the callee can *still* check against nil, though.
	return 0;
}

int y8_foreach(lua_State *state) {
	// stack[1]: table
	// stack[2]: function

	// PICO-8 edge cases for foreach:
	// - `foreach()` is a noop (so no checking needed, because #nil will be 0)
	// - `foreach({"foo"})` causes an error (so no checking needed)
	// - `foreach({"foo"}, 1)` causes an error (so no checking needed)

	// Metatable safety notes:
	// - This does not respect the metatable of the table for the length
	//   operator or for the index operator
	// - This should respect the metatable for item comparison that is used to
	//   allow deletion of the iterated element during foreach

	const auto len = int(lua_rawlen(state, 1));

	for (int i = 1; i <= len;) {
		lua_rawgeti(state, 1, i); // push table[i]

		if (!lua_isnil(state, -1)) {
			// save/push current value of table[i] for later check
			lua_pushvalue(state, -1);

			// prepare call to function(table[i]):
			// push function
			lua_pushvalue(state, 2);

			// push value
			lua_pushvalue(state, -2);

			// stack[-3]: value
			// stack[-2]: function
			// stack[-1]: value
			lua_call(state, 1, 0);

			// now:
			// stack[-1]: value

			// standard PICO-8 accomodates the deranged usecase of calling del
			// into a table being foreach'd by, apparently, checking whether the
			// item at the current position got modified.

			// lookup values (avoid pushing)
			const TValue *table = index2addr(state, 1);
			const TValue *old_value = index2addr(state, -1);
			const TValue *new_value = luaH_getint(hvalue(table), i);

			if (!equalobj(state, old_value, new_value)) {
				lua_settop(state, -2); // pop value
				continue;              // skip incrementing
			}
			lua_settop(state, -2); // pop value
		} else {
			lua_settop(state, -2); // pop the value pushed by rawgeti
		}

		++i;
	}

	return 0;
}

namespace {

/// Sets lua_stack[-1][element_index] to the provided string, eventually parsing
/// to a number if `parse_numeric` is true and if the number can actually be
/// parsed as one.
void set_split_element(lua_State *state, int element_index,
                       std::string_view str, bool parse_numeric) {
	lua_Number parsed_number;
	int is_number = 0; // int because of lua_tonumberx, but functionally a bool

	lua_pushlstring(state, str.data(), str.size());

	if (parse_numeric) {
		// we cannot use luaO_str2d directly because it assumes the input string
		// is nul-terminated at the correct size, and otherwise reads past the
		// end.
		// the one we push on the stack is fine
		parsed_number = lua_tonumberx(state, -1, &is_number);
	}

	// stack[-1]: string
	if (is_number) {
		// replace stack[-1] with the number instead
		lua_settop(state, -2);
		lua_pushnumber(state, parsed_number);
	}

	// stack[-2]: table to push this to
	// stack[-1]: string or number to set at index element_index
	lua_rawseti(state, -2, element_index);
}

} // namespace

int y8_split(lua_State *state) {
	// stack[1]: str
	// stack[2] (optional default ",")
	//          - separator str
	//          - OR integer (to split every N chars)
	// stack[3] (optional default true): boolean, whether to parse numbers

	const auto arg_count = lua_gettop(state);

	if (arg_count < 1) {
		// PICO-8 returns no value in that case
		return 0;
	}

	std::string_view input;
	{ // read input into a string_view
		std::size_t input_size;
		const char *input_buf = lua_tolstring(state, 1, &input_size);
		input = {input_buf, input_size};
	}

	bool is_numeric_separator = false;
	bool parse_numeric = true;
	std::size_t table_size = 0;

	std::size_t split_every;    // if is_number_mode
	std::string_view separator; // if !is_number_mode

	// parse separator argument or assume default
	if (arg_count >= 2) {
		if (lua_type(state, 2) == LUA_TSTRING) { // read into a string_view
			std::size_t sep_size;
			const char *sep_buf = lua_tolstring(state, 2, &sep_size);
			separator = {sep_buf, sep_size};
		} else {
			is_numeric_separator = true;
		}
	} else {
		// assume default separator
		separator = ",";
	}

	// parse argument that decides whether to parse numbers
	if (arg_count >= 3) {
		parse_numeric = lua_toboolean(state, 3);
	}

	// first pass: determine the final array size.
	if (is_numeric_separator) {
		split_every = std::max(1, lua_tointeger(state, 2));
		// divide and round up, maybe should move this to a common math header
		table_size =
			(input.size() / split_every) + (input.size() % split_every != 0);
	} else {
		table_size = 1;

		std::size_t i = 0;
		while (i < input.size() - separator.size()) {
			if (input.substr(i, separator.size()) == separator) {
				i += separator.size();
				++table_size;
			} else {
				++i;
			}
		}
	}

	// second pass: create table with known size and fill it
	lua_createtable(state, int(table_size), 0);
	// stack[-1] is the table to return

	if (is_numeric_separator) {
		std::size_t remaining = input.size();
		for (std::size_t i = 0; i < table_size; ++i) {
			std::string_view substr{input.data() + i * split_every,
			                        std::min(split_every, remaining)};
			set_split_element(state, int(i + 1), substr, parse_numeric);
			remaining -= substr.size();
		}
	} else {
		int split_idx = 1;
		std::size_t current_substr_offset = 0;
		std::size_t i = 0;
		while (i < input.size()) {
			if (input.substr(i, separator.size()) == separator) {
				set_split_element(state, split_idx,
				                  input.substr(current_substr_offset,
				                               i - current_substr_offset),
				                  parse_numeric);
				i += separator.size();
				++split_idx;
				current_substr_offset = i;
			} else {
				++i;
			}
		}

		// write final split (as it won't have triggered a separator above)
		// we have to do this even if this is of a length of 0
		set_split_element(state, split_idx, input.substr(current_substr_offset),
		                  parse_numeric);
	}

	return 1;
}

int y8_unpack(lua_State *state) {
	// stack[1]: table
	// stack[2] (optional default 1): first index to unpack
	// stack[3] (optional default #tbl): last index to unpack

	const auto arg_count = lua_gettop(state);

	int start_index = 1;
	if (arg_count >= 2) {
		start_index = lua_tointeger(state, 2);
	}

	int end_index;
	if (arg_count >= 3) {
		end_index = lua_tointeger(state, 3);
	} else {
		end_index = int(lua_rawlen(state, 1));
	}

	for (int i = start_index; i <= end_index; ++i) {
		lua_rawgeti(state, 1, i);
	}

	return end_index - start_index + 1;
}

} // namespace emu::bindings