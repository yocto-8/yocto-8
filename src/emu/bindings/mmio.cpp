#include "mmio.hpp"
#include "hal/hal.hpp"
#include "hal/types.hpp"
#include "p8/parser.hpp"

#include <coredefs.hpp>
#include <emu/emulator.hpp>
#include <lauxlib.h>
#include <lua.h>

using namespace y8;

namespace emu::bindings {

namespace {

/// \brief Accesses the memory byte at the provided address, wrapping around to
/// `0x0000` (as the parameter is u16).
u8 &byte_at(PicoAddr addr) { return emu::emulator.memory().data[addr]; }

} // namespace

int y8_peek(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const PicoAddr base = luaL_checkunsigned(state, 1);

	std::size_t n = 1;
	if (argument_count >= 2) {
		n = std::min(luaL_checkunsigned(state, 2), 8192u);
	}

	for (std::size_t i = 0; i < n; ++i) {
		lua_pushunsigned(state, byte_at(base + i));
	}

	return int(n);
}

int y8_peek2(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const PicoAddr base = luaL_checkunsigned(state, 1);

	std::size_t n = 1;
	if (argument_count >= 2) {
		n = std::min(luaL_checkunsigned(state, 2), 8192u);
	}

	for (std::size_t i = 0; i < n; ++i) {
		const PicoAddr addr = base + i * 2;
		const u16 dword = byte_at(addr + 0) << 0 | byte_at(addr + 1) << 8;
		lua_pushunsigned(state, int(dword));
	}

	return int(n);
}

int y8_peek4(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const PicoAddr base = luaL_checkunsigned(state, 1);

	std::size_t n = 1;
	if (argument_count >= 2) {
		n = std::min(luaL_checkunsigned(state, 2), 8192u);
	}

	for (std::size_t i = 0; i < n; ++i) {
		const PicoAddr addr = base + i * 4;
		const u32 qword = byte_at(addr + 0) << 0 | byte_at(addr + 1) << 8 |
		                  byte_at(addr + 2) << 16 | byte_at(addr + 3) << 24;
		lua_pushnumber(state, LuaFix16::from_fix16(int(qword)));
	}

	return int(n);
}

int y8_poke(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const PicoAddr base = luaL_checkunsigned(state, 1);

	for (int i = 0; i < argument_count - 1; ++i) {
		// first parameter is arg 1, first value is arg 2
		byte_at(base + i) = luaL_checkunsigned(state, i + 2);
	}

	return 0;
}

int y8_poke2(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const PicoAddr base = luaL_checkunsigned(state, 1);

	for (int i = 0; i < argument_count - 1; ++i) {
		const auto addr = base + i * 4;
		const u16 value = luaL_checkunsigned(state, i + 2);
		byte_at(addr + 0) = (value >> 0) & 0xFF;
		byte_at(addr + 1) = (value >> 8) & 0xFF;
	}

	return 0;
}

int y8_poke4(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const PicoAddr base = luaL_checkunsigned(state, 1);

	for (int i = 0; i < argument_count - 1; ++i) {
		const auto addr = base + i * 4;
		const RawFix16 value = luaL_checknumber(state, i + 2).value;
		byte_at(addr + 0) = (value >> 0) & 0xFF;
		byte_at(addr + 1) = (value >> 8) & 0xFF;
		byte_at(addr + 2) = (value >> 16) & 0xFF;
		byte_at(addr + 3) = (value >> 24) & 0xFF;
	}

	return 0;
}

int y8_memcpy(lua_State *state) {
	const auto dst = luaL_checkunsigned(state, 1);
	const auto src = luaL_checkunsigned(state, 2);
	const auto len = luaL_checkunsigned(state, 3);

	// FIXME: this should properly fix src/dst/len values
	emulator.memory().memcpy(dst, src, len);

	return 0;
}

int y8_memset(lua_State *state) {
	const auto dst = luaL_checkunsigned(state, 1);
	const std::uint8_t val = luaL_checkunsigned(state, 2);
	const auto len = luaL_checkunsigned(state, 3);

	emulator.memory().memset(dst, val, len);

	return 0;
}

int y8_reload(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	p8::ParserMapConfig config{
		.target_region_start = y8::PicoAddr(lua_tounsigned(state, 1)),
		.source_region_start = y8::PicoAddr(lua_tounsigned(state, 2)),
		.region_size = argument_count >= 3 ? lua_tounsigned(state, 3) : 0x4300,
		.clear_memory = true};

	config.disable_state(p8::ParserState::PARSING_LUA);

	int cart_name_stack_pos;
	if (argument_count >= 4) {
		cart_name_stack_pos = 4;
	} else {
		lua_getglobal(state, "__cart_name");
		cart_name_stack_pos = -1;
	}

	std::string_view cart_path;
	{
		std::size_t cart_path_len;
		const char *cart_path_buf =
			lua_tolstring(state, cart_name_stack_pos, &cart_path_len);
		cart_path = {cart_path_buf, cart_path_len};
	}

	// __cart_name still on the stack; doesn't matter, will get cleared

	if (config.source_region_start != 0x0000 ||
	    config.target_region_start != 0x0000 || config.region_size < 0x4300) {
		printf("STUB reload(): Currently not supporting non-default memory "
		       "region; this might cause corruption!\n");
	}

	hal::FileReaderContext reader;

	const hal::FileOpenStatus status =
		hal::fs_create_open_context(cart_path, reader);

	if (status != hal::FileOpenStatus::SUCCESS) {
		// FIXME: needs some form of debug here that's better than this
		printf("reload from '%.*s' failed, skipping\n", int(cart_path.size()),
		       cart_path.data());
		return 0;
	}

	p8::parse(hal::fs_read_buffer, &reader, config);

	hal::fs_destroy_open_context(reader);
	return 0;
}

int y8_load(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	std::string_view cart_path;
	{
		std::size_t cart_path_len;
		const char *cart_path_buf = lua_tolstring(state, 1, &cart_path_len);
		cart_path = {cart_path_buf, cart_path_len};
	}

	if (cart_path != "") {
		// will throw
		emu::emulator.trigger_load_from_vm(cart_path);
	}

	return 0;
}

} // namespace emu::bindings