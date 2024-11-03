#include "time.hpp"

#include <emu/emulator.hpp>
#include <lauxlib.hpp>
#include <lua.hpp>

#include <hal/hal.hpp>

namespace emu::bindings {

int y8_time(lua_State *state) {
	const auto time_us = hal::measure_time_us();

	// TODO: this could be done more efficiently and accurately(?) than through
	// the double conversion

	const LuaFix16 time_s(double(time_us) / 1.0e6);

	lua_pushnumber(state, time_s);
	return 1;
}

} // namespace emu::bindings