#include <cstdio>
#include <hal/hal.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <string>

#include <sys/time.h>

#include "../window.hpp"
#include "hal/types.hpp"
#include <emu/emulator.hpp>
#include <unistd.h>

namespace hal {

std::uint16_t update_button_state() {
#ifndef Y8_DESKTOP_HEADLESS
	return arch::desktop::yolo_window.button_state;
#else
	return 0;
#endif
}

void present_frame() {
#ifndef Y8_DESKTOP_HEADLESS
	arch::desktop::yolo_window.present_frame(
		emu::device<devices::Framebuffer>, emu::device<devices::ScreenPalette>);
#endif
}

static std::uint64_t timer_start_micros;

// TODO: headless: time accounting should be less shite

void reset_timer() { timer_start_micros = measure_time_us(); }

std::uint64_t measure_time_us() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (1000000 * std::uint64_t(tv.tv_sec) + std::uint64_t(tv.tv_usec)) -
	       timer_start_micros;
}

void delay_time_us(std::uint64_t time) { usleep(time); }

void load_rgb_palette(
	[[maybe_unused]] std::span<std::uint32_t, 32> new_palette) {
	printf("RGB palette update unimplemented on desktop\n");
}

std::span<const std::uint32_t, 32> get_default_palette() {
	return video::pico8_palette_rgb8;
}

std::span<char> read_repl(std::span<char> target_buffer) {
	// int byte_count = 0;
	// if ((fseek(stdin, 0, SEEK_END), ftell(stdin)) > 0) {
	// 	std::string out_str;
	// 	std::getline(std::cin, out_str);
	// 	std::size_t size_bytes =
	// 		std::min(out_str.size() + 1, target_buffer.size());

	// 	std::memcpy(target_buffer.data(), out_str.data(), size_bytes);

	// 	return target_buffer.subspan(0, size_bytes);
	// }
	return {};
}

FileOpenStatus fs_create_open_context(std::string_view path,
                                      FileReaderContext &ctx) {
	ctx.file = fopen(std::string(path).c_str(), "rb");
	return ctx.file != nullptr ? FileOpenStatus::SUCCESS : FileOpenStatus::FAIL;
}

void fs_destroy_open_context(FileReaderContext &ctx) { fclose(ctx.file); }

const char *fs_read_buffer(void *context, std::size_t *size) {
	FileReaderContext &real_context =
		*static_cast<FileReaderContext *>(context);

	*size = fread(real_context.buf.data(), 1, real_context.buf.size(),
	              real_context.file);
	return real_context.buf.data();
}

std::uint32_t get_unique_seed() { return rand(); }

} // namespace hal