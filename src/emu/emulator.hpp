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

class Emulator {
	public:
	constexpr Emulator() = default;
	~Emulator();

	void init(std::span<std::byte> memory_buffer);

	void load_and_inject_header(hal::ReaderCallback *cart_reader, void *ud);
	void exec(std::string_view buf);

	void handle_repl();

	void run();
	void flip();

	int get_fps_target() const;

	enum class HookResult { SUCCESS, UNDEFINED, LUA_ERROR };

	HookResult run_hook(const char *name);

	[[noreturn]] void panic(const char *message);

	constexpr Memory memory() { return Memory{std::span(_memory)}; }

	auto get_memory_alloc_buffer() const { return _memory_buffer; }

	auto &palette() { return _palette; }

	std::uint64_t get_update_start_time() { return _update_start_time; }
	std::uint64_t get_frame_start_time() { return _frame_start_time; }
	std::uint64_t get_frame_target_time() { return _frame_target_time; }

	Emulator(const Emulator &) = delete;
	Emulator &operator=(const Emulator &) = delete;

	private:
	std::span<std::byte> _memory_buffer;
	std::array<std::uint8_t, 65536> _memory = {};
	std::array<std::uint32_t, 32> _palette = {};
	lua_State *_lua = nullptr;

	std::uint64_t _update_start_time = 0;
	std::uint64_t _frame_start_time = 0;
	std::uint64_t _frame_target_time = 0;
};

// void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize);

extern constinit Emulator emulator;

template <class Device, std::uint16_t map_address = Device::default_map_address>
inline constexpr auto device =
	Device(emulator.memory().data.subspan<map_address, Device::map_length>());

} // namespace emu