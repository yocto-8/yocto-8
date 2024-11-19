#include <cstdio>
#include <hal/hal.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <string>

#include <sys/time.h>

#include "../window.hpp"
#include "hal/types.hpp"
#include <dirent.h>
#include <emu/emulator.hpp>
#include <unistd.h>

namespace hal {

ButtonState update_button_state() {
#ifndef Y8_DESKTOP_HEADLESS
	arch::desktop::yolo_window.dispatch_tick_events();
	return {.held_key_mask = arch::desktop::yolo_window.held_key_mask,
	        .pressed_key_mask = arch::desktop::yolo_window.pressed_key_mask};
#else
	return {0, 0};
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

std::uint64_t absolute_timer_value() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (1000000 * std::uint64_t(tv.tv_sec) + std::uint64_t(tv.tv_usec));
}

void reset_timer() { timer_start_micros = absolute_timer_value(); }

std::uint64_t measure_time_us() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return absolute_timer_value() - timer_start_micros;
}

void post_frame_hooks() {}

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

void fs_set_working_directory(std::string_view path) {
	chdir(std::string(path).c_str());
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

	if (*size == 0) {
		return nullptr;
	}

	return real_context.buf.data();
}

DirectoryListStatus fs_list_directory(DirectoryListCallback *callback, void *ud,
                                      const char *path) {
	DIR *d = opendir(path);
	if (d == NULL) {
		return DirectoryListStatus::FAIL;
	}

	dirent *dir;
	while ((dir = readdir(d)) != nullptr) {
		std::string_view name = dir->d_name;

		if (name == "." || name == "..") {
			continue;
		}

		FileInfo out_info{.kind = (dir->d_type != DT_DIR
		                               ? FileInfo::Kind::FILE
		                               : FileInfo::Kind::DIRECTORY),
		                  .name = name};

		callback(ud, out_info);
	}
	closedir(d); // finally close the directory
}

std::uint32_t get_unique_seed() { return rand(); }

} // namespace hal