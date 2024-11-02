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
#include <ldo.h>
#include <lua.h>
#include <lualib.h>

#include "y8header.hpp"
#include "y8std.hpp"
#include <devices/buttonstate.hpp>
#include <devices/clippingrectangle.hpp>
#include <devices/drawpalette.hpp>
#include <devices/screenpalette.hpp>
#include <hal/hal.hpp>
#include <p8/parser.hpp>

namespace emu {

Emulator::~Emulator() {
	if (_lua != nullptr) {
		lua_close(_lua);
	}
}

void Emulator::init(std::span<std::byte> backup_heap_buffer) {
	_backup_heap = backup_heap_buffer;

	const auto default_palette = hal::get_default_palette();
	std::copy(default_palette.begin(), default_palette.end(), _palette.begin());

	_lua = lua_newstate(y8_lua_realloc, &_backup_heap, _memory.data());

#ifdef Y8_EXPERIMENTAL_GENGC
	// printf("Buggy Lua 5.2 Generational GC enabled\n");
	luaC_changemode(_lua, KGC_GEN);
#endif
	hal::load_rgb_palette(_palette);

	// Recreate global table with 128 preallocated hashmap records
	lua_createtable(_lua, 0, 128);
	lua_rawseti(_lua, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

	bind_globals();

	_memory.fill(0);
	device<devices::DrawPalette>.reset();
	device<devices::DrawStateMisc>.reset();
	device<devices::ScreenPalette>.reset();
	device<devices::ClippingRectangle>.reset();
	device<devices::Random>.set_seed(hal::get_unique_seed());
	_button_state = {};
}

void Emulator::bind_globals() {
	luaL_openlibs(_lua);

	const auto stub = [&](const char *name) {
		lua_register(_lua, name, [](lua_State *) {
			// printf("unimplemented blahblah\n");
			return 0;
		});
	};

	for (const auto &binding : y8_std) {
		lua_register(_lua, binding.name, binding.callback);
	}

	for (const auto &binding : y8_numeric_globals) {
		lua_pushnumber(_lua, binding.value);
		lua_setglobal(_lua, reinterpret_cast<const char *>(binding.name));
	}

	stub("music");
	stub("sfx");
}

void Emulator::set_active_cart_path(std::string_view cart_path) {
	lua_pushlstring(_lua, cart_path.data(), cart_path.size());
	lua_setglobal(_lua, "__cart_name");
}

void Emulator::trigger_load_from_vm(std::string_view cart_path) {
	if (cart_path.size() >= _persistent_state.load_path_cstr.size() - 1) {
		panic("Path to load() is too long");
	}

	std::memcpy(_persistent_state.load_path_cstr.data(), cart_path.data(),
	            cart_path.size());
	_persistent_state.load_path_cstr[cart_path.size()] = '\0';

	throw EmulatorResetRequest();
}

bool Emulator::load_from_path(std::string_view cart_path) {
	printf("Loading cart from \"%.*s\"\n", int(cart_path.size()),
	       cart_path.data());
	set_active_cart_path(cart_path);

	if (const auto last_dash_pos = cart_path.find_last_of("/\\");
	    last_dash_pos != cart_path.npos) {

		hal::fs_set_working_directory(cart_path.substr(0, last_dash_pos));
	}

	hal::FileReaderContext reader;

	const hal::FileOpenStatus status =
		hal::fs_create_open_context(cart_path, reader);

	if (status != hal::FileOpenStatus::SUCCESS) {
		return false;
	}

	if (const auto parse_status =
	        p8::parse(hal::fs_read_buffer, &reader, {}, _lua->l_G);
	    parse_status != p8::ParserStatus::OK) {
		printf("Load from '%.*s' failed with error code %d!\n",
		       int(cart_path.size()), cart_path.data(), int(parse_status));
		return false;
	}

	hal::fs_destroy_open_context(reader);

	return true;
}

void Emulator::load_and_inject_header(Reader reader) {
	CartImporterReader state{.header_reader{app_header}, .cart_reader = reader};

	const int load_status = lua_load(
		_lua,
		[]([[maybe_unused]] lua_State *state, void *ud, size_t *sz) {
			return CartImporterReader::reader_callback(ud, sz);
		},
		&state, "/bios/header.lua", "t");

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

void Emulator::run_until_shutdown() {
	for (;;) {
		try {
			run_once();
			break;
		} catch (const EmulatorResetRequest &e) {
			lua_close(_lua);
			init(_backup_heap);
			load_from_path(_persistent_state.load_path_cstr.data());
		}
	}
}

void Emulator::run_once() {
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

		_button_state = hal::update_button_state();
		device<devices::ButtonState>.for_player(0) =
			_button_state.held_key_mask;

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

	hal::post_frame_hooks();

	const auto taken_time = hal::measure_time_us() - _frame_start_time;

	if (taken_time < _frame_target_time) {
		hal::delay_time_us(_frame_target_time - taken_time);
	}

	_frame_start_time = hal::measure_time_us();
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
	for (;;) {
		handle_repl();
		hal::present_frame();
	}
#else
	exit(1);
#endif
}

Emulator emulator;

} // namespace emu