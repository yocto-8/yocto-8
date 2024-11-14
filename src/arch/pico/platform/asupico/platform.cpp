#include "devices/screenpalette.hpp"
#include "emu/emulator.hpp"
#include "emu/tlsf.hpp"
#include "fs/types.hpp"
#include <arch/pico/fs/hwinit.hpp>
#include <arch/pico/usb/hwinit.hpp>
#include <cstdio>
#include <emu/alloc.hpp>
#include <pico/flash.h>
#include <platform/asupico/asupico.hpp>
#include <platform/platform.hpp>

#include <hardwarestate.hpp>
#include <video/palette.hpp>

// provided by linker script
extern "C" {
extern char __heap_start, __heap_end;
extern char __psram_start, __psram_heap_start;
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
	printf("  Heap#1 size: %d bytes\n", &__heap_end - &__heap_start);

	fast_heap =
		tlsf_create_with_pool(&__heap_start, &__heap_end - &__heap_start);
	printf("%p %p\n", &__heap_start, &__heap_end - &__heap_start);

	printf("Configuring frequency and clock divisors\n");
	init_flash_frequency();
	init_default_frequency();
	printf("Configuring PSRAM\n");
	// we must do this sort of early as C dynamic allocs are broken before PSRAM
	// is initialized, although we keep those to virtually 0 outside of lua
	const auto psram_size = init_psram_pimoroni();
	const auto psram_reserved = &__psram_heap_start - &__psram_start;
	release_assert(psram_size > 0);
	printf("   PSRAM size: %d bytes\n", psram_size);
	const std::size_t heap_size = psram_size - psram_reserved;
	printf("  Heap#2 size: %d bytes (buffers: %d)\n", heap_size,
	       psram_reserved);
	slow_heap_span = std::span{&__psram_heap_start, heap_size};
	slow_heap = tlsf_create_with_pool(&__psram_heap_start, heap_size);
	printf("Configuring GPIO\n");
	init_basic_gpio();
	printf("Configuring video\n");
	// init_video_ssd1351();
	init_video_dwo();
	printf("Booting command thread\n");
	init_cmd_thread(); // should be done after most hw init
	printf("Configuring FatFS (flash)\n");
	init_flash_fatfs(); // should be done after command thread init (for flash
	                    // lock mechanism reasons)
	printf("Initializing USB\n");
	init_usb_device();
	printf("Configuring emulator\n");
	emu::emulator.init();
	printf("Hardware init done\n");
}

void present_frame(FrameCopiedCallback *callback) {
	// asupico::hw.ssd1351.copy_framebuffer(emu::device<devices::Framebuffer>,
	//                                      emu::device<devices::ScreenPalette>);

	asupico::state::dwo.copy_framebuffer(emu::device<devices::Framebuffer>,
	                                     emu::device<devices::ScreenPalette>);

	if (callback != nullptr) {
		callback();
	}

	// asupico::hw.ssd1351.start_scanout();
	// asupico::hw.dwo.start_scanout();
}

void local_core_init() {
	// asupico::hw.ssd1351.init_dma_on_this_core();
	asupico::state::dwo.init_dma_on_this_core();
}

std::span<const std::uint32_t, 32> get_default_palette() {
	return ::video::pico8_palette_rgb8;
}

} // namespace arch::pico::platform