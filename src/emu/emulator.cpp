#include "emulator.hpp"
#include "devices/drawstatemisc.hpp"
#include "devices/image.hpp"
#include "devices/random.hpp"
#include "emu/bufferio.hpp"
#include "lgc.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <lauxlib.hpp>
#include <ldo.hpp>
#include <lua.hpp>
#include <lualib.hpp>

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
	if (_lua_ready) {
		lua_close(lua());
	}
}

void Emulator::init() {
	const auto default_palette = hal::get_default_palette();
	std::copy(default_palette.begin(), default_palette.end(), _palette.begin());

	lua_newstate(y8_lua_realloc, nullptr, &_lua_preallocated_state,
	             _memory.data());
	_lua_ready = true;

#ifdef Y8_EXPERIMENTAL_GENGC
	// printf("Buggy Lua 5.2 Generational GC enabled\n");
	luaC_changemode(lua(), KGC_GEN);
#endif
	hal::load_rgb_palette(_palette);

	// Recreate global table with 128 preallocated hashmap records
	lua_createtable(lua(), 0, 128);
	lua_rawseti(lua(), LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

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
	luaL_openlibs(lua());

	const auto stub = [&](const char *name) {
		lua_register(lua(), name, [](lua_State *) {
			// printf("unimplemented blahblah\n");
			return 0;
		});
	};

	for (const auto &binding : y8_std) {
		lua_register(lua(), binding.name, binding.callback);
	}

	for (const auto &binding : y8_numeric_globals) {
		lua_pushnumber(lua(), binding.value);
		lua_setglobal(lua(), reinterpret_cast<const char *>(binding.name));
	}

	stub("music");
	stub("sfx");
}

void Emulator::set_active_cart_path(std::string_view cart_path) {
	lua_pushlstring(lua(), cart_path.data(), cart_path.size());
	lua_setglobal(lua(), "__cart_name");
}

void Emulator::trigger_load_from_vm(std::string_view cart_path) {
	if (cart_path.size() >= _persistent_state.load_path_cstr.size() - 1) {
		panic("Path to load() is too long");
	}

	std::memcpy(_persistent_state.load_path_cstr.data(), cart_path.data(),
	            cart_path.size());
	_persistent_state.load_path_cstr[cart_path.size()] = '\0';

	throw EmulatorReset();
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

	if (const auto parse_status = p8::parse(hal::fs_read_buffer, &reader, {},
	                                        &_lua_preallocated_state.g);
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
		lua(),
		[]([[maybe_unused]] lua_State *state, void *ud, size_t *sz) {
			return CartImporterReader::reader_callback(ud, sz);
		},
		&state, "/bios/header.lua", "t");

	if (load_status != 0) {
		printf("Script load failed: %s\n", lua_tostring(lua(), -1));
		panic(lua_tostring(lua(), -1));
		lua_pop(lua(), 1);
	}
}

void Emulator::exec(std::string_view buf) {
	const int load_status =
		luaL_loadbuffer(lua(), buf.data(), buf.size(), "repl");

	if (load_status != 0) {
		printf("Exec compilation failed: %s\n", lua_tostring(lua(), -1));
		panic(lua_tostring(lua(), -1));
		lua_pop(lua(), 1);
	}

	if (lua_pcall(lua(), 0, 0, 0) != 0) {
		printf("Exec failed: %s\n", lua_tostring(lua(), -1));
		panic(lua_tostring(lua(), -1));
		lua_pop(lua(), 1);
	}
}

void Emulator::handle_repl() {
#ifdef Y8_LUA_REPL
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	std::array<char, 512> repl_buffer;
	const auto repl_input = hal::read_repl(repl_buffer);

	if (!repl_input.empty()) {
		exec(std::string_view{repl_input.data(), repl_input.size()});
	}
#endif
}

void Emulator::run_until_shutdown() {
	// This loop makes use of exceptions to handle loading and panics.
	//
	// This is kinda hacky for a few reasons, including:
	// - attempting to reduce stack space usage
	// - avoiding unbounded recursion like panic->repl->panic->etc
	// - proper exception handling for handle_repl and such

	for (;;) {
		try {
			run_once();
			break;
		} catch (const EmulatorReset &e) {
			goto reset_handler;
		} catch (const EmulatorPanic &e) {
			goto panic_handler;
		}

	reset_handler: {
		lua_close(lua());
		init();
		try {
			load_from_path(_persistent_state.load_path_cstr.data());
		} catch (...) {
		}
		continue;
	}

	panic_handler: {
		for (;;) {
			try {
				handle_repl();
			} catch (const EmulatorReset &e) {
				goto reset_handler;
			} catch (...) {
			}
			flip();
		}
	}
	}
}

void Emulator::run_once() {
	hal::reset_timer();

	if (lua_pcall(lua(), 0, 0, 0) != 0) {
		printf("Script exec at load time failed: %s\n",
		       lua_tostring(lua(), -1));
		panic(lua_tostring(lua(), -1));
		lua_pop(lua(), 1);
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
	lua_gc(lua(), LUA_GCSTEP, 0);
#else
	lua_gc(lua(), LUA_GCSTEP, 100);
#endif

	hal::post_frame_hooks();

	const auto taken_time = hal::measure_time_us() - _frame_start_time;

	if (taken_time < _frame_target_time) {
		hal::delay_time_us(_frame_target_time - taken_time);
	}

	_frame_start_time = hal::measure_time_us();
}

int Emulator::get_fps_target() const {
	// this function is const wrt the resulting state of the emulator
	auto *lua_state = const_cast<Emulator *>(this)->lua(); // NOLINT

	lua_getglobal(lua_state, "_update60");
	const bool is60 = lua_isfunction(lua_state, -1);
	lua_pop(lua_state, 1);

	return is60 ? 60 : 30;
}

Emulator::HookResult Emulator::run_hook(const char *name) {
	lua_getglobal(lua(), name);

	if (!lua_isfunction(lua(), -1)) {
		lua_pop(lua(), 1);
		return HookResult::UNDEFINED;
	}

	if (lua_pcall(lua(), 0, 0, 0) != 0) {
		printf("hook '%s' execution failed: %s\n", name,
		       lua_tostring(lua(), -1));
		panic(lua_tostring(lua(), -1));
		lua_pop(lua(), 1);
		// return HookResult::LUA_ERROR;
	}

	return HookResult::SUCCESS;
}

void Emulator::panic(const char *message) {
	device<devices::DrawPalette>.reset();
	device<devices::ScreenPalette>.reset();
	device<devices::DrawStateMisc>.reset();
	device<devices::Framebuffer>.clear(0);
	device<devices::DrawStateMisc>.set_text_point({4, 4});

	printf("PANIC: %s\n", message);

	device<devices::DrawStateMisc>.raw_pen_color() = 7;
	lua_getglobal(lua(), "print");
	lua_pushstring(lua(), "unrecoverable error");
	lua_pcall(lua(), 1, 0, 0);

	device<devices::DrawStateMisc>.raw_pen_color() = 6;
	lua_getglobal(lua(), "print");
	lua_pushstring(lua(), message);
	lua_pcall(lua(), 1, 0, 0);

	hal::present_frame();

#ifdef Y8_INFINITE_LOOP_EXIT
	// TODO: optional breakpoint?
	throw EmulatorPanic();
#else
	exit(1);
#endif
}

Emulator emulator; // NOLINT

} // namespace emu