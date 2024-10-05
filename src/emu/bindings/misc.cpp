#include "fix16.h"
#include "hal/hal.hpp"
#include "lobject.h"

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <emu/emulator.hpp>
#include <lauxlib.h>
#include <lua.h>

namespace emu::bindings {

enum class StatEntry : std::uint16_t {
	RAM_USAGE_KB = 0,
	CPU_USAGE_SINCE_UPDATE = 1,
	SYSCALL_USAGE_SINCE_UPDATE = 2,
	MAP_DISPLAY = 3,
	CLIPBOARD_STRING = 4,
	P8_VERSION = 5,
	LOAD_PARAMETER = 6,
	CURRENT_FPS = 7,
	TARGET_FPS = 8,
	SYSTEM_FPS = 9,
	// 10 is unknown/reserved
	ENABLED_DISPLAY_COUNT = 11,
	PAUSE_MENU_UL_X = 12,
	PAUSE_MENU_UL_Y = 13,
	PAUSE_MENU_BR_X = 14,
	PAUSE_MENU_BR_Y = 15,
	// TODO: deprecated audio stuff 16..26
	// 27 is unknown/reserved
	RAW_KEYBOARD = 28, // with 2nd scancode param
	// 29 is unknown
	// TODO: devkit mouse/keyboard entries/stubs 30..39
	// TODO: music/sound 46..56
	// 57..71 is unknown, but we could match nil/empty str behavior
	// TODO: time of day 80..95
	PRE_GC_RAM_USAGE_KB = 99,
	// TODO: 100..102 BBS
	// 103..107 is unknown
	// TODO: 110 is "frame-by-frame mode flag"
	// 111..119 is unknown
	// TODO: 120..121 GPIO functionality thingie
	// 122..?? is unknown
};

int y8_stat(lua_State *state) {
	[[maybe_unused]] const auto argument_count = lua_gettop(state);

	const auto entry = lua_tointeger(state, 1);

	// FIXME: this is a stub implementation.
	// some games use this for meaningful things, and some of the entries return
	// strings.

	switch (StatEntry(entry)) {
	case StatEntry::RAM_USAGE_KB: {
		// p8 allegedly GCs before stat(0)
		// ... but let's not do that, this is particularly expensive...
		// lua_gc(state, LUA_GCCOLLECT, 0);

		const auto usage_kb_part = lua_gc(state, LUA_GCCOUNT, 0);
		const auto usage_b_part = lua_gc(state, LUA_GCCOUNTB, 0);
		lua_pushnumber(state,
		               LuaFix16(std::int16_t(usage_kb_part), usage_b_part));
		break;
	}

	// no distinction between stat(1) and stat(2), since we aren't emulating
	// cycles in the first place
	case StatEntry::CPU_USAGE_SINCE_UPDATE:
	case StatEntry::SYSCALL_USAGE_SINCE_UPDATE: {
		const auto current_time = hal::measure_time_us();

		// let's compute this with floats -- not sure if fixed point would
		// overflow
		const auto delta_time =
			float(current_time - emu::emulator.get_update_start_time());
		const auto budget_spent =
			delta_time / float(emu::emulator.get_frame_target_time());

		lua_pushnumber(state, LuaFix16(budget_spent));

		break;
	}

	default: {
		// unhandled; return 0 by default
		lua_pushnumber(state, LuaFix16());
		return 1;
	}
	}

	return 1;
}

int y8_exit(lua_State *state) { std::exit(lua_tounsigned(state, 1)); }

int y8_gc(lua_State *state) {
	lua_gc(state, LUA_GCCOLLECT, 0);
	return 0;
}

int y8_printh(lua_State *state) {
	// based on lua standard print()
	// FIXME: i don't think standard printh in pico-8 uses tostring.

	int n = lua_gettop(state); /* number of arguments */

	// PICO-8 does *not* print a newline on empty input
	if (n == 0) {
		return 0;
	}

	int i;
	lua_getglobal(state, "tostring");
	for (i = 1; i <= n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(state, -1); /* function to be called */
		lua_pushvalue(state, i);  /* value to print */
		lua_call(state, 1, 1);
		s = lua_tolstring(state, -1, &l); /* get result */
		if (s == nullptr) {
			return luaL_error(
				state,
				LUA_QL("tostring") " must return a string to " LUA_QL("print"));
		}
		if (i > 1) {
			printf("\t");
		}

		printf("%s", s);
		lua_pop(state, 1); /* pop result */
	}
	printf("\n");
	return 0;
}

enum class ToStrFlagOffsets {
	HEX_OR_ID = 0,
	AS_PLAIN_INTEGER = 1,
};

/// \brief Format unique hex address with PICO-8 `tostr` semantics, given a
/// prefix
void format_address(std::span<char> buffer, std::string_view prefix,
                    const void *address) {
	std::memcpy(buffer.data(), prefix.data(), prefix.size());
	const auto output_span = buffer.subspan(prefix.size());

	[[maybe_unused]] const auto err =
		std::snprintf(output_span.data(), output_span.size(), "0x%" PRIxPTR,
	                  std::uintptr_t(address));
	assert(err > 0);
}

int tostr_handle_optional_address_mode(lua_State *state,
                                       const char *on_normal_literal,
                                       std::string_view on_address_str,
                                       bool should_use_address_format) {
	if (!should_use_address_format) {
		lua_pushstring(state, on_normal_literal);
		return 1;
	}

	// address format
	std::array<char, 32> char_buffer;
	format_address(char_buffer, on_address_str, lua_topointer(state, 1));
	lua_pushstring(state, char_buffer.data());
	return 1;
}

void format_number(std::span<char> buffer, LuaFix16 number, bool as_hex,
                   bool as_integral) {
	assert(buffer.size() > 16); // rough sanity check

	if (as_integral) {
		if (as_hex) {
			[[maybe_unused]] const int err = std::snprintf(
				buffer.data(), buffer.size(), "0x%08x", number.value);
			assert(err > 0);
		} else {
			[[maybe_unused]] const int err =
				std::snprintf(buffer.data(), buffer.size(), "%d", number.value);
			assert(err > 0);
		}

		return;
	}

	if (as_hex) {
		[[maybe_unused]] const int err = std::snprintf(
			buffer.data(), buffer.size(), "0x%04x.%04x",
			number.unsigned_integral_bits(), number.decimal_bits());
		assert(err > 0);
	} else {
		if (number.decimal_bits() != 0) {
			// TODO: maybe move this out of the fix16 lib
			fix16_to_str(number.value, buffer.data(), 4);
		} else {
			[[maybe_unused]] const int err =
				std::snprintf(buffer.data(), buffer.size(), "%d",
			                  number.signed_integral_bits());
			assert(err > 0);
		}
	}
}

int y8_tostr(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	// PICO-8 just returns an empty string when no arg is passed
	if (argument_count == 0) {
		lua_pushliteral(state, "");
		return 1;
	}

	// boolean doesn't care about flags whatsoever: always false or true
	if (lua_isboolean(state, 1)) {
		// can't be a ternary operator as it cares about the literal length
		if (lua_toboolean(state, 1)) {
			lua_pushliteral(state, "true");
		} else {
			lua_pushliteral(state, "false");
		}
		return 1;
	}

	// string doesn't care about flags whatsoever, return itself
	// lua_isstring returns true for numbers, which are implicitly convertible
	if (lua_type(state, 1) == LUA_TSTRING) {
		// TODO: can this possibly be returned directly instead?
		lua_pushstring(state, lua_tostring(state, 1));
		return 1;
	}

	// nil doesn't care about flags whatsoever: always [nil]
	if (lua_isnil(state, 1)) {
		lua_pushliteral(state, "[nil]");
		return 1;
	}

	int flags = 0;
	bool has_flags_specified = false;

	if (argument_count >= 2) {
		if (lua_isboolean(state, 2)) {
			flags = lua_toboolean(state, 2) ? 0b1 : 0;
		} else {
			flags = lua_tounsigned(state, 2);
		}
		has_flags_specified = true;
	}

	// the current PICO-8 behavior is that whenever the flags argument is
	// passed, then functions/tables/etc. switch to the address syntax,
	// regardless of the value (not just & 0b01).
	const bool should_use_address_format = has_flags_specified;

	const bool hex_or_id_mode =
		(flags >> int(ToStrFlagOffsets::HEX_OR_ID)) & 0b1;
	const bool as_plain_integer_mode =
		(flags >> int(ToStrFlagOffsets::AS_PLAIN_INTEGER)) & 0b1;

	if (lua_isfunction(state, 1)) {
		return tostr_handle_optional_address_mode(
			state, "[function]", "function: ", should_use_address_format);
	}

	if (lua_istable(state, 1)) {
		return tostr_handle_optional_address_mode(
			state, "[table]", "table: ", should_use_address_format);
	}

	if (lua_isthread(state, 1)) {
		return tostr_handle_optional_address_mode(
			state, "[thread]", "thread: ", should_use_address_format);
	}

	if (lua_isnumber(state, 1)) {
		std::array<char, 32> number_buffer;
		format_number(number_buffer, lua_tonumber(state, 1), hex_or_id_mode,
		              as_plain_integer_mode);
		lua_pushstring(state, number_buffer.data());
		return 1;
	}

	return luaL_error(state, "Unexpected input to tostr()");
}

int y8_tonum(lua_State *state) {
	// args bound checking is not required:
	// empty or non-string will result in no return by default

	std::string_view input;
	{ // read input into a string_view
		std::size_t input_size;
		const char *input_buf = lua_tolstring(state, 1, &input_size);
		input = {input_buf, input_size};
	}

	unsigned flags = lua_tounsigned(state, 2);

	const bool is_hex = (flags >> 0) & 0b1;
	const bool is_shifted = (flags >> 1) & 0b1;
	const bool is_return_zero_on_fail = (flags >> 2) & 0b1;

	int isnum = 0; // to treat as boolean
	int parse_mask = LPARSE_ALLOW_EXPONENT;
	if (is_hex) {
		parse_mask |= LPARSE_HEX;
	}
	if (is_shifted) {
		parse_mask |= LPARSE_SHIFT;
	}
	const lua_Number number = lua_tonumberx(state, 1, &isnum, parse_mask);

	if (isnum || is_return_zero_on_fail) {
		lua_pushnumber(state, number);
		return 1;
	}

	return 0;
}

// based on lua standard sub()
static size_t posrelat(ptrdiff_t pos, size_t len) {
	if (pos >= 0)
		return (size_t)pos;
	else if (0u - (size_t)pos > len)
		return 0;
	else
		return len - ((size_t)-pos) + 1;
}

int y8_sub(lua_State *L) {
	size_t l;
	const char *s = luaL_checklstring(L, 1, &l);
	size_t start = posrelat(luaL_checkinteger(L, 2), l);
	size_t end = posrelat(luaL_optinteger(L, 3, -1), l);
	if (start < 1)
		start = 1;
	if (end > l)
		end = l;
	if (start <= end)
		lua_pushlstring(L, s + start - 1, end - start + 1);
	else
		lua_pushliteral(L, "");
	return 1;
}

} // namespace emu::bindings