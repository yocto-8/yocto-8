#include "mmio.hpp"

#include <emu/emulator.hpp>
#include <lauxlib.h>
#include <lua.h>

namespace emu::bindings {

namespace {
std::uint8_t &byte_at(std::size_t addr) {
	return emu::emulator.memory().data[addr % 65536];
}
} // namespace

int y8_peek(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const auto addr = luaL_checkunsigned(state, 1);

	std::size_t n = 1;
	if (argument_count >= 2) {
		n = std::min(luaL_checkunsigned(state, 2), 8192u);
	}

	for (std::size_t i = 0; i < n; ++i) {
		lua_pushunsigned(state, byte_at(addr + i));
	}

	return int(n);
}

int y8_peek2(lua_State *state) {
	const auto addr = luaL_checkunsigned(state, 1);
	const std::uint16_t dword = byte_at(addr + 0) | byte_at(addr + 1) << 8;
	lua_pushunsigned(state, dword);
	return 1;
}

int y8_peek4(lua_State *state) {
	const auto addr = luaL_checkunsigned(state, 1);
	const std::uint32_t qword =
		byte_at(addr + 0) << 0 | byte_at(addr + 1) << 8 |
		byte_at(addr + 2) << 16 | byte_at(addr + 3) << 24;
	lua_pushnumber(state, LuaFix16::from_fix16(int(qword)));
	return 1;
}

int y8_poke(lua_State *state) {
	const auto argument_count = lua_gettop(state);

	const auto addr = luaL_checkunsigned(state, 1);

	for (int i = 0; i < argument_count - 1; ++i) {
		// first parameter is arg 1, first value is arg 2
		byte_at(addr + i) = luaL_checkunsigned(state, i + 2);
	}

	return 0;
}

int y8_poke2(lua_State *state) {
	const auto addr = luaL_checkunsigned(state, 1);
	const auto value = luaL_checkunsigned(state, 2);
	byte_at(addr + 0) = (value >> 0) & 0xFF;
	byte_at(addr + 1) = (value >> 8) & 0xFF;
	return 0;
}

int y8_poke4(lua_State *state) {
	const auto addr = luaL_checkunsigned(state, 1);
	const auto value = luaL_checknumber(state, 2).value;
	byte_at(addr + 0) = (value >> 0) & 0xFF;
	byte_at(addr + 1) = (value >> 8) & 0xFF;
	byte_at(addr + 2) = (value >> 16) & 0xFF;
	byte_at(addr + 3) = (value >> 24) & 0xFF;
	return 0;
}

int y8_memcpy(lua_State *state) {
	const auto dst = luaL_checkunsigned(state, 1);
	const auto src = luaL_checkunsigned(state, 2);
	const auto len = luaL_checkunsigned(state, 3);

	// FIXME: this should properly fix src/dst/len values
	// should this or Memory::memcpy perform bounds checking?
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

} // namespace emu::bindings