#include "pico/time.h"
#include <cstdio>
#include <hal/hal.hpp>

#include <cmdthread.hpp>
#include <hardwarestate.hpp>
#include <pico/rand.h>
#include <pico/stdio.h>

namespace hal {

namespace pico = arch::pico;

// NOTE: some of the implementations are deferred to the platform.

void present_frame() {
	pico::run_blocking_command(arch::pico::IoThreadCommand::PUSH_FRAME);
}

void reset_timer() { pico::state.timer_start = get_absolute_time(); }

std::uint64_t measure_time_us() {
	const auto current_time = get_absolute_time();
	return absolute_time_diff_us(pico::state.timer_start, current_time);
}

void delay_time_us(std::uint64_t time) { sleep_us(time); }

std::span<char> read_repl(std::span<char> target_buffer) {
	// FIXME: at current this hangs both the picosystem and my setup if there is
	// nothing receiving

	// int c = stdio_getchar_timeout_us(0);

	// if (c == PICO_ERROR_TIMEOUT || c == '\n' || c == '\r') {
	// 	return {};
	// }

	// std::size_t i = 0;
	// do {
	// 	target_buffer[i] = c;
	// 	c = getchar();
	// 	++i;
	// } while (c != '\r' && c != '\n');

	// return {target_buffer.data(), i};
	return {};
}

// filesystem stubs

FileOpenStatus fs_create_open_context(std::string_view path,
                                      FileReaderContext &ctx) {
	return FileOpenStatus::FAIL;
}

void fs_destroy_open_context(FileReaderContext &ctx) {}

const char *fs_read_buffer(void *context, std::size_t *size) { return nullptr; }

std::uint32_t get_unique_seed() { return std::uint32_t(get_rand_32()); }

} // namespace hal