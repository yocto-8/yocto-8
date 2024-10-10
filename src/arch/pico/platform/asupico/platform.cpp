#include "fs/types.hpp"
#include <arch/pico/fs/hwinit.hpp>
#include <arch/pico/usb/hwinit.hpp>
#include <cstdio>
#include <emu/tinyalloc.hpp>
#include <pico/flash.h>
#include <platform/asupico/asupico.hpp>
#include <platform/platform.hpp>

#include <hardwarestate.hpp>
#include <video/palette.hpp>

// provided by linker script
extern "C" {
extern char __heap_start, __heap_end;
extern char __flash_binary_start, __flash_binary_end;
}

namespace arch::pico::platform {

void init_hardware() {
	using namespace asupico;

	init_stdio();

	printf("   Flash size: %d bytes\n", PICO_FLASH_SIZE_BYTES);
	printf("Firmware size: %d bytes (reserved: %d)\n", y8_firmware_size,
	       Y8_RESERVED_FIRMWARE_SIZE);
	release_assert(y8_firmware_size < Y8_RESERVED_FIRMWARE_SIZE);
	printf("Int. FAT size: %d bytes\n", y8_fatfs_size);
	printf("    Heap size: %d bytes\n", &__heap_end - &__heap_start);

	printf("Configuring frequency and clock divisors\n");
	init_flash_frequency();
	init_default_frequency();
	printf("Configuring PSRAM\n");
	const auto psram_size = init_psram_pimoroni();
	printf("Configuring PSRAM heap\n");
	ta_init();
	printf("PSRAM configured with size %dKiB\n", psram_size / 1024);
	printf("Configuring GPIO\n");
	init_basic_gpio();
	printf("Configuring video\n");
	init_video_ssd1351();
	printf("Booting command thread\n");
	init_cmd_thread(); // should be done after most hw init
	printf("Configuring FatFS (flash)\n");
	init_flash_fatfs(); // should be done after command thread init (for flash
	                    // lock mechanism reasons)
	printf("Initializing USB\n");
	init_usb_device();
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

void local_core_init() { asupico::hw.ssd1351.init_dma_on_this_core(); }

std::span<const std::uint32_t, 32> get_default_palette() {
	return ::video::ssd1351_precal_palette_rgb8;
}

} // namespace arch::pico::platform