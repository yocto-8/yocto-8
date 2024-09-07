#include <cstdio>
#include <platform/asupico/asupico.hpp>
#include <platform/platform.hpp>

#include <hardwarestate.hpp>
#include <video/palette.hpp>

namespace arch::pico::platform {

void init_hardware() {
	using namespace asupico;

	init_stdio();
	printf("Configuring frequency and clock divisors\n");
	init_flash_frequency();
	init_default_frequency();
	printf("Configuring PSRAM\n");
	const auto psram_size = init_psram_pimoroni();
	printf("PSRAM configured with size %dKiB\n", psram_size / 1024);
	printf("Booting command thread\n");
	init_cmd_thread();
	printf("Configuring buttons\n");
	init_buttons();
	printf("Configuring video\n");
	init_video_ssd1351();
	printf("Configuring emulator\n");
	init_emulator(psram_size);
	printf("Hardware init done\n");
}

void present_frame(FrameCopiedCallback *callback) {
	asupico::hw.ssd1351.copy_framebuffer(emu::device<devices::Framebuffer>,
	                                     emu::device<devices::ScreenPalette>);

	if (callback != nullptr) {
		callback();
	}

	asupico::hw.ssd1351.start_scanout();
}

std::span<const std::uint32_t, 32> get_default_palette() {
	return ::video::ssd1351_precal_palette_rgb8;
}

} // namespace arch::pico::platform