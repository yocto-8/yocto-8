#pragma once

#include <array>
#include <lua.h>
#include <span>
#include <string_view>

#include <emu/bufferio.hpp>
#include <emu/mmio.hpp>
#include <hal/hal.hpp>
#include <video/palette.hpp>

namespace emu {

struct EmulatorPersistentState {
	std::array<char, 128> load_path_cstr;
};

class Emulator {
	public:
	constexpr Emulator() = default;
	~Emulator();

	void init(std::span<std::byte> backup_heap_buffer);
	void bind_globals();

	void set_active_cart_path(std::string_view cart_path);

	void trigger_load_from_vm(std::string_view cart_path);
	bool load_from_path(std::string_view cart_path);
	void load_and_inject_header(Reader reader);
	void exec(std::string_view buf);

	void handle_repl();

	void run_until_shutdown();
	void run_once();
	void flip();

	int get_fps_target() const;

	enum class HookResult { SUCCESS, UNDEFINED, LUA_ERROR };

	HookResult run_hook(const char *name);

	[[noreturn]] void panic(const char *message);

	constexpr Memory memory() { return Memory{std::span(_memory)}; }

	auto get_backup_heap_buffer() const { return _backup_heap; }

	auto &palette() { return _palette; }

	std::uint64_t get_update_start_time() { return _update_start_time; }
	std::uint64_t get_frame_start_time() { return _frame_start_time; }
	std::uint64_t get_frame_target_time() { return _frame_target_time; }

	Emulator(const Emulator &) = delete;
	Emulator &operator=(const Emulator &) = delete;

	private:
	EmulatorPersistentState _persistent_state;
	std::span<std::byte> _backup_heap;
	std::array<std::uint8_t, 65536> _memory;
	std::array<std::uint32_t, 32> _palette;
	lua_State *_lua = nullptr;

	std::uint64_t _update_start_time = 0;
	std::uint64_t _frame_start_time = 0;
	std::uint64_t _frame_target_time = 0;
};

// void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize);

extern Emulator emulator;

template <class Device, std::uint16_t map_address = Device::default_map_address>
inline constexpr auto device =
	Device(emulator.memory().data.subspan<map_address, Device::map_length>());

} // namespace emu

#include "alloc.hpp"