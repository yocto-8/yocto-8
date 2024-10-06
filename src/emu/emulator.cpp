#include "emulator.hpp"
#include "devices/drawstatemisc.hpp"
#include "devices/image.hpp"
#include "devices/random.hpp"
#include "emu/bufferio.hpp"
#include "lgc.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <devices/buttonstate.hpp>
#include <devices/clippingrectangle.hpp>
#include <devices/drawpalette.hpp>
#include <devices/screenpalette.hpp>
#include <emu/bindings/input.hpp>
#include <emu/bindings/math.hpp>
#include <emu/bindings/misc.hpp>
#include <emu/bindings/mmio.hpp>
#include <emu/bindings/rng.hpp>
#include <emu/bindings/table.hpp>
#include <emu/bindings/time.hpp>
#include <emu/bindings/video.hpp>
#include <hal/hal.hpp>

namespace emu {

// TODO: minify lua header
/// @brief This header is injected at the start of every cart.
/// Whatever symbols it brings to the `local` scope will be visible to the cart.
static constexpr std::string_view app_header =
	R"(
local all = function(t)
	if t == nil or #t == 0 then
		return function() end
	end

	local i = 1
	local prev = nil

	return function()
		if t[i] == prev then
			i += 1
		end

		while t[i] == nil and i <= #t do
			i += 1
		end

		prev = t[i]

		return prev
	end
end

local count = function(t, v)
	local n = 0
	if v == nil then
		for i = 1,#t do
			if t[i] ~= nil then
				n += 1
			end
		end
	else
		for i = 1,#t do
			if t[i] == v then
				n += 1
			end
		end
	end
	return n
end

function __panic(msg)
	printh("PANIC: " .. msg)
	print(":(", 0, 0, 7)
	print(msg)
end
)"

	/// This re-exports certain globals into the `local` scope of the cart.
    /// Calling these is faster, but there is a 200 limit on locals (including
    /// the cart's own), so it shouldn't be used a ton. That limit only affects
    /// the number of locals that are used, AFAICT.
    ///
    /// This should preferably only re-export standard functions that are used
    /// in hot loops.
    ///
    /// When re-exported in the `local` scope, any use gets added to the upvalue
    /// table of a function. Usually, this is a net performance benefit, because
    /// upvalue accesses can be done directly rather than through an environment
    /// lookup by string key (even if this usecase was optimized for in y8).
    ///
    /// However, it also results in longer upvalue construction time
    /// per-function... For how carts normally behave, this is probably fine.

	R"(local color, pset, pget, sset, sget, fget, line, circfill, rectfill, spr, sspr, pal, palt, fillp, clip, mset, mget, map, peek, peek2, peek4, poke, poke2, poke4, memcpy, memset, abs, flr, mid, min, max, sin, cos, sqrt, shl, shr, band, bor, rnd, t, time, add, foreach, split, unpack, ord =
color, pset, pget, sset, sget, fget, line, circfill, rectfill, spr, sspr, pal, palt, fillp, clip, mset, mget, map, peek, peek2, peek4, poke, poke2, poke4, memcpy, memset, abs, flr, mid, min, max, sin, cos, sqrt, shl, shr, band, bor, rnd, t, time, add, foreach, split, unpack, ord
)"

	R"(_gc(); printh(stat(0) .. "KB at boot"))";

using BindingCallback = int(lua_State *);

struct Binding {
	/// @brief Name to export the binding under.
	const char *name;

	/// @brief Reference to the callback. Always of this fixed signature.
	BindingCallback &callback;
};

static constexpr std::array<Binding, 63> y8_std{{
	{"camera", bindings::y8_camera},
	{"color", bindings::y8_color},
	{"pset", bindings::y8_pset},
	{"pget", bindings::y8_pget},
	{"sset", bindings::y8_sset},
	{"sget", bindings::y8_sget},
	{"fget", bindings::y8_fget},
	{"cls", bindings::y8_cls},
	{"line", bindings::y8_line},
	{"circfill", bindings::y8_circfill},
	{"rectfill", bindings::y8_rectfill},
	{"spr", bindings::y8_spr},
	{"sspr", bindings::y8_sspr},
	{"pal", bindings::y8_pal},
	{"palt", bindings::y8_palt},
	{"fillp", bindings::y8_fillp},
	{"clip", bindings::y8_clip},
	{"mset", bindings::y8_mset},
	{"mget", bindings::y8_mget},
	{"map", bindings::y8_map},
	{"flip", bindings::y8_flip},
	{"print", bindings::y8_print},
	{"_rgbpal", bindings::y8_rgbpal},

	{"btn", bindings::y8_btn},

	{"peek", bindings::y8_peek},
	{"peek2", bindings::y8_peek2},
	{"peek4", bindings::y8_peek4},
	{"poke", bindings::y8_poke},
	{"poke2", bindings::y8_poke2},
	{"poke4", bindings::y8_poke4},
	{"memcpy", bindings::y8_memcpy},
	{"memset", bindings::y8_memset},
	{"reload", bindings::y8_reload},

	{"abs", bindings::y8_abs},
	{"flr", bindings::y8_flr},
	{"mid", bindings::y8_mid},
	{"min", bindings::y8_min},
	{"max", bindings::y8_max},
	{"sin", bindings::y8_sin},
	{"cos", bindings::y8_cos},
	{"sqrt", bindings::y8_sqrt},
	{"shl", bindings::y8_shl},
	{"shr", bindings::y8_shr},
	{"band", bindings::y8_band},
	{"bor", bindings::y8_bor},

	{"cursor", bindings::y8_cursor},
	{"printh", bindings::y8_printh},
	{"tostr", bindings::y8_tostr},
	{"tonum", bindings::y8_tonum},
	{"stat", bindings::y8_stat},
	{"sub", bindings::y8_sub},
	{"ord", bindings::y8_ord},
	{"_exit", bindings::y8_exit},
	{"_gc", bindings::y8_gc},

	{"rnd", bindings::y8_rnd},
	{"srand", bindings::y8_srand},

	{"t", bindings::y8_time},
	{"time", bindings::y8_time},

	{"add", bindings::y8_add},
	{"del", bindings::y8_del},
	{"foreach", bindings::y8_foreach},
	{"split", bindings::y8_split},
	{"unpack", bindings::y8_unpack},
}};

Emulator::~Emulator() {
	if (_lua != nullptr) {
		lua_close(_lua);
	}
}

void Emulator::init(std::span<std::byte> memory_buffer) {
	_memory_buffer = memory_buffer;

	const auto default_palette = hal::get_default_palette();
	std::copy(default_palette.begin(), default_palette.end(), _palette.begin());

	_lua = lua_newstate(y8_lua_realloc, &_memory_buffer, _memory.data());

#ifdef Y8_EXPERIMENTAL_GENGC
	printf("Buggy Lua 5.2 Generational GC enabled\n");
	luaC_changemode(_lua, KGC_GEN);
#endif
	luaL_openlibs(_lua);

	device<devices::DrawPalette>.reset();
	device<devices::DrawStateMisc>.reset();
	device<devices::ScreenPalette>.reset();
	device<devices::ClippingRectangle>.reset();
	device<devices::Random>.set_seed(hal::get_unique_seed());

	const auto bind = [&](const char *name, const auto &func) {
		lua_pushcfunction(_lua, func);
		lua_setglobal(_lua, name);
	};

	const auto stub = [&](const char *name) {
		bind(name, [](lua_State *) {
			// printf("unimplemented blahblah\n");
			return 0;
		});
	};

	for (const auto &binding : y8_std) {
		lua_pushcfunction(_lua, binding.callback);
		lua_setglobal(_lua, binding.name);
	}

	stub("music");
	stub("sfx");

	hal::load_rgb_palette(_palette);
}

void Emulator::set_active_cart_path(std::string_view cart_path) {
	lua_pushlstring(_lua, cart_path.data(), cart_path.size());
	lua_setglobal(_lua, "__cart_name");
}

void Emulator::load_and_inject_header(Reader reader) {
	CartImporterReader state{.header_reader{app_header}, .cart_reader = reader};

	const int load_status = lua_load(
		_lua,
		[]([[maybe_unused]] lua_State *state, void *ud, size_t *sz) {
			return CartImporterReader::reader_callback(ud, sz);
		},
		&state, "m", "t");

	if (load_status != 0) {
		printf("Script load failed: %s\n", lua_tostring(_lua, -1));
		panic(lua_tostring(_lua, -1));
		lua_pop(_lua, 1);
	}
}

void Emulator::exec(std::string_view buf) {
	const int load_status =
		luaL_loadbuffer(_lua, buf.data(), buf.size(), "repl");

	if (load_status != 0) {
		printf("Exec compilation failed: %s\n", lua_tostring(_lua, -1));
		panic(lua_tostring(_lua, -1));
		lua_pop(_lua, 1);
	}

	if (lua_pcall(_lua, 0, 0, 0) != 0) {
		printf("Exec failed: %s\n", lua_tostring(_lua, -1));
		panic(lua_tostring(_lua, -1));
		lua_pop(_lua, 1);
	}
}

void Emulator::handle_repl() {
#ifdef Y8_LUA_REPL
	std::array<char, 512> repl_buffer;
	const auto repl_input = hal::read_repl(repl_buffer);

	if (!repl_input.empty()) {
		exec(std::string_view{repl_input.data(), repl_input.size()});
	}
#endif
}

void Emulator::run() {
	hal::reset_timer();

	if (lua_pcall(_lua, 0, 0, 0) != 0) {
		printf("Script exec at load time failed: %s\n", lua_tostring(_lua, -1));
		panic(lua_tostring(_lua, -1));
		lua_pop(_lua, 1);
	}

	_frame_target_time = 1'000'000u / get_fps_target();

	run_hook("_init");

	// if no hook ends up being executed in the loop; then assume we should be
	// exiting
	bool has_any_hook_defined;

	do {
		has_any_hook_defined = false;

		/// execute a hook and updates has_any_hook_defined for this iteration
		const auto run_loop_hook = [&](const char *name) {
			const auto hook_result = run_hook(name);
			has_any_hook_defined |= hook_result != HookResult::UNDEFINED;
			return hook_result;
		};

		handle_repl();

		// this _update behavior matches pico-8's: if _update60 is defined,
		// _update is ignored when both are unspecified, flipping will occur at
		// 30Hz regardless

		// we however don't clip the framerate to either 15/30/60. just push the
		// frames as early as we can.

		// TODO: need a mechanism to occasionally flip when draw is never called
		// (e.g. billion pale dots do that for a loading screen)

		_update_start_time = hal::measure_time_us();

		if (run_loop_hook("_update60") == HookResult::UNDEFINED) {
			run_loop_hook("_update");
		}

		run_loop_hook("_draw");

		flip();
	} while (has_any_hook_defined);
}

void Emulator::flip() {
	hal::present_frame();

	// printf("%f\n", double(taken_time) / 1000.0);

#ifdef Y8_EXPERIMENTAL_GENGC
	lua_gc(_lua, LUA_GCSTEP, 0);
#else
	lua_gc(_lua, LUA_GCSTEP, 100);
#endif

	const auto taken_time = hal::measure_time_us() - _frame_start_time;

	if (taken_time < _frame_target_time) {
		hal::delay_time_us(_frame_target_time - taken_time);
	}

	_frame_start_time = hal::measure_time_us();

	device<devices::ButtonState>.for_player(0) = hal::update_button_state();
}

int Emulator::get_fps_target() const {
	lua_getglobal(_lua, "_update60");
	const bool is60 = lua_isfunction(_lua, -1);
	lua_pop(_lua, 1);

	return is60 ? 60 : 30;
}

Emulator::HookResult Emulator::run_hook(const char *name) {
	lua_getglobal(_lua, name);

	if (!lua_isfunction(_lua, -1)) {
		lua_pop(_lua, 1);
		return HookResult::UNDEFINED;
	}

	if (lua_pcall(_lua, 0, 0, 0) != 0) {
		printf("hook '%s' execution failed: %s\n", name,
		       lua_tostring(_lua, -1));
		panic(lua_tostring(_lua, -1));
		lua_pop(_lua, 1);
		// return HookResult::LUA_ERROR;
	}

	return HookResult::SUCCESS;
}

void Emulator::panic(const char *message) {
	device<devices::DrawPalette>.reset();
	device<devices::ScreenPalette>.reset();
	// device<devices::DrawStateMisc>.reset();
	device<devices::Framebuffer>.clear(0);

	lua_getglobal(_lua, "__panic");
	lua_pushstring(_lua, message);

	lua_pcall(_lua, 1, 0, 0);

	hal::present_frame();

#ifdef Y8_INFINITE_LOOP_EXIT
	for (;;)
		;
#else
	exit(1);
#endif
}

constinit Emulator emulator;

} // namespace emu