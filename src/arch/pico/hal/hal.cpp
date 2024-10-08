#include "emu/bufferio.hpp"
#include "pico/time.h"
#include <cstdio>
#include <hal/hal.hpp>
#include <hal/types.hpp>

#include "../main/bios.hpp"
#include <cmdthread.hpp>
#include <hardware/pwm.h>
#include <hardwarestate.hpp>
#include <pico/rand.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>

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

void delay_time_us(std::uint64_t time) {
	// squared to compensate for non-linear power vs brightness curve
	pwm_set_gpio_level(PICO_DEFAULT_LED_PIN, 0);
	sleep_us(time);
	pwm_set_gpio_level(PICO_DEFAULT_LED_PIN,
	                   Y8_LED_PWM_SCALE * Y8_LED_PWM_SCALE);
}

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
	if (path == "/y8/bios.p8") {
		ctx.is_bios_read = true;
		ctx.reader.bios_reader = emu::StringReader(bios_cartridge);
		return FileOpenStatus::SUCCESS;
	}

	return FileOpenStatus::FAIL;
}

void fs_destroy_open_context(FileReaderContext &ctx) {
	// no-op for now
	(void)ctx;
}

const char *fs_read_buffer(void *context, std::size_t *size) {
	FileReaderContext &real_context =
		*static_cast<FileReaderContext *>(context);

	if (real_context.is_bios_read) {
		emu::StringReader &bios_reader = real_context.reader.bios_reader;
		return bios_reader.get_reader()(size);
	}

	return nullptr;
}

std::uint32_t get_unique_seed() { return std::uint32_t(get_rand_32()); }

} // namespace hal