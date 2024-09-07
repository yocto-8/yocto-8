#include "ssd1351.hpp"
#include "devices/image.hpp"
#include "devices/screenpalette.hpp"
#include <cstdio>
#include <hardware/dma.h>

namespace arch::pico::video {

[[gnu::section(Y8_SRAM_SECTION)]] static void ssd1351_global_dma_handler() {
	SSD1351::active_instance->scanline_dma_update();
}

[[gnu::cold]] void SSD1351::init(Config config) {
	_spi = config.spi;
	_pinout = config.pinout;
	_current_dma_fb_offset = 0;

	gpio_set_function(_pinout.sclk, GPIO_FUNC_SPI);
	gpio_set_function(_pinout.tx, GPIO_FUNC_SPI);

	gpio_init(_pinout.rst);
	gpio_set_dir(_pinout.rst, GPIO_OUT);
	gpio_put(_pinout.rst, 1);

	gpio_init(_pinout.cs);
	gpio_set_dir(_pinout.cs, GPIO_OUT);
	gpio_put(_pinout.cs, 1);

	gpio_init(_pinout.dc);
	gpio_set_dir(_pinout.dc, GPIO_OUT);
	gpio_put(_pinout.dc, 0);

	reset_blocking();
	_submit_init_sequence();

	_dma_channel = dma_claim_unused_channel(true);
	_configure_dma_channel();
}

[[gnu::cold]] void SSD1351::shutdown() {
	// TODO: cancel any in-flight IRQ, tear down/unclaim DMA, bring any GPIO to
	// defaults/unselected, etc.
}

[[gnu::cold]] void SSD1351::_submit_init_sequence() {
	// Enable MCU interface (else some commands will be dropped)
	write(Command::SET_COMMAND_LOCK, DataBuffer<1>{0x12});
	write(Command::SET_COMMAND_LOCK, DataBuffer<1>{0xB1});

	// Shut off the display while we set it up
	write(Command::DISPLAY_OFF);

	// Set clock division stuff and cover the entire screen
	write(Command::SET_CLOCK_DIVIDER, DataBuffer<1>{0xF0});
	write(Command::SET_MUX_RATIO, DataBuffer<1>{127});

	// 64K 16-bit format, enable split, CBA, correct rotation
	write(Command::SET_REMAP, DataBuffer<1>{0b01110100});

	// Set display offsets
	write(Command::SET_START_LINE, DataBuffer<1>{0});
	write(Command::SET_DISPLAY_OFFSET, DataBuffer<1>{0});

	// Disable both GPIO pins
	write(Command::SET_GPIO, DataBuffer<1>{0x00});

	// Set SPI interface, disable sleep mode
	write(Command::SET_FUNCTION, DataBuffer<1>{0b00'1});

	// Timing tweaks
	write(Command::SET_PRECHARGE_PERIOD, DataBuffer<1>{0b0010'0010});
	write(Command::SET_PRECHARGE2_PERIOD, DataBuffer<1>{0b0001});

	// Voltage tweaks
	write(Command::SET_PRECHARGE_VOLTAGE, DataBuffer<1>{0b11111});
	write(Command::SET_COM_DESELECT_VOLTAGE, DataBuffer<1>{0b111});

	// Seems to be useless?
	// write(Command::MAGIC_ENHANCE_DISPLAY, DataBuffer<3>{0xA4, 0x00,
	// 0x00});

	// Display regular pixel data
	write(Command::NORMAL_DISPLAY);

	// Contrast settings
	write(Command::SET_CHANNEL_CONTRAST, DataBuffer<3>{0xFF, 0xFF, 0xFF});
	set_brightness(0xF);
	// const float gamma = 1.25;
	// set_brightness(0xA); const float gamma = 1.0;
	// set_brightness(0x5); const float gamma = 0.75;
	// set_brightness(0x3); const float gamma = 0.7;
	// set_brightness(0x2); const float gamma = 0.6;

	// Set cryptic command from the datasheet that does fuck knows
	write(Command::SET_VSL, DataBuffer<3>{0xA0, 0xB5, 0x55});

	write(Command::SET_GRAYSCALE_LUT, gamma_lut);

	// Start display
	write(Command::DISPLAY_ON);
}

[[gnu::cold]] void SSD1351::_configure_dma_channel() {
	dma_channel_config dma_cfg = dma_channel_get_default_config(_dma_channel);

	// TODO: is DMA_SIZE_32 (with transfer_count edited accordingly) supposed to
	// be scuffed?
	channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
	channel_config_set_dreq(&dma_cfg, spi_get_dreq(_spi, true));

	// read increment + no write increment is the default
	channel_config_set_read_increment(&dma_cfg, true);
	channel_config_set_write_increment(&dma_cfg, false);

	constexpr std::size_t scanline_buffer_size_bytes = 128 * 2;
	dma_channel_configure(_dma_channel, &dma_cfg, &spi_get_hw(_spi)->dr,
	                      nullptr, scanline_buffer_size_bytes, false);

	// configure DMA IRQ and configure it to trigger when DMA scanned out a line
	dma_channel_set_irq0_enabled(_dma_channel, true);
	irq_set_exclusive_handler(DMA_IRQ_0, ssd1351_global_dma_handler);
	irq_set_enabled(DMA_IRQ_0, true);
}

[[gnu::section(Y8_SRAM_SECTION)]]
void SSD1351::scanline_dma_update() {
	// OK to call even if the function wasn't triggered by IRQ (i.e. on first
	// run)
	dma_channel_acknowledge_irq0(_dma_channel);

	if (_current_dma_fb_offset == devices::Framebuffer::frame_bytes) {
		// final DMA scanline transfered, deselect chip and return
		_current_dma_fb_offset = 0;
		gpio_put(_pinout.cs, 1);
		return;
	}

	unsigned current_fb_scanline_end =
		_current_dma_fb_offset + 64; // 128 half-bytes

	const auto screen_palette = emu::device<devices::ScreenPalette>;
	const auto fb = emu::device<devices::Framebuffer>;

	for (std::size_t fb_idx = _current_dma_fb_offset, scanline_idx = 0;
	     fb_idx < current_fb_scanline_end; ++fb_idx, scanline_idx += 2) {
		_scanline_buffer[scanline_idx + 0] =
			palette[screen_palette.get_color(fb.data[fb_idx] & 0x0F)];
		_scanline_buffer[scanline_idx + 1] =
			palette[screen_palette.get_color(fb.data[fb_idx] >> 4)];
	}

	_current_dma_fb_offset = current_fb_scanline_end;

	// trigger new scanline transfer; we will get IRQ'd again when scaned out
	dma_channel_set_read_addr(_dma_channel, _scanline_buffer.data(), true);
}

[[gnu::flatten, gnu::section(Y8_SRAM_SECTION)]]
void SSD1351::start_scanout() {
	active_instance = this;

	if (_current_dma_fb_offset != 0) {
		// Scanout was still going on when we flipped, which will result in
		// tearing.
		// This is not really supposed to happen, but it isn't catastrophic.
		// Return from this; let it finish the frame.
		return;
	}

	// SRAM writes should cover all the framebuffer (0..127)
	write(Command::SET_COLUMN, DataBuffer<2>{0, 127});
	write(Command::SET_ROW, DataBuffer<2>{0, 127});

	// Start write
	write(Command::RAM_WRITE);

	gpio_put(_pinout.dc, 1);
	gpio_put(_pinout.cs, 0);

	// trigger the first scanline write manually. IRQs will trigger the rest
	scanline_dma_update();
}

SSD1351 *SSD1351::active_instance;

} // namespace arch::pico::video
