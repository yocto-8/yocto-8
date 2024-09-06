#include "pico/time.h"
#include "video/palette.hpp"
#include <cstdio>
#include <hal/hal.hpp>

#include <cmdthread.hpp>
#include <hardwarestate.hpp>
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
	int c = stdio_getchar_timeout_us(0);

	if (c == PICO_ERROR_TIMEOUT || c == '\n' || c == '\r') {
		return {};
	}

	std::size_t i = 0;
	do {
		target_buffer[i] = c;
		c = getchar();
		++i;
	} while (c != '\r' && c != '\n');

	return {target_buffer.data(), i};
}

} // namespace hal