#include "dwoqspi.hpp"
#include "devices/image.hpp"
#include "devices/screenpalette.hpp"
#include <cstdio>
#include <hardware/dma.h>

namespace arch::pico::video {

[[gnu::section(Y8_SRAM_SECTION), gnu::flatten, gnu::noinline]] void
dwo_global_dma_handler() {
	// DWO::active_instance->write(
	// 	DWO::Command::SET_COLUMN,
	// 	DWO::DataBuffer<4>{DWO::x_start >> 8, DWO::x_start & 0xFF,
	//                        DWO::x_end >> 8, DWO::x_end & 0xFF});
	// DWO::active_instance->write(
	// 	DWO::Command::SET_ROW,
	// 	DWO::DataBuffer<4>{DWO::y_start >> 8, DWO::y_start & 0xFF,
	//                        DWO::y_end >> 8, DWO::y_end & 0xFF});
	// DWO::active_instance->write(DWO::Command::RAM_WRITE);
	DWO::active_instance->scanline_dma_update();
}

[[gnu::cold]] void DWO::init(Config config) {
	_spi = config.spi;
	_pinout = config.pinout;
	_current_dma_fb_offset = 0;

	gpio_set_function(_pinout.sclk, GPIO_FUNC_SPI);
	gpio_set_function(_pinout.sio0, GPIO_FUNC_SPI);

	gpio_init(_pinout.rst);
	gpio_set_dir(_pinout.rst, GPIO_OUT);
	gpio_put(_pinout.rst, 1);

	gpio_init(_pinout.cs);
	gpio_set_dir(_pinout.cs, GPIO_OUT);
	gpio_put(_pinout.cs, 1);

	gpio_init(_pinout.pwr_en);
	gpio_set_dir(_pinout.pwr_en, GPIO_OUT);
	gpio_put(_pinout.pwr_en, 0);

	// gpio_init(_pinout.dc);
	// gpio_set_dir(_pinout.dc, GPIO_OUT);
	// gpio_put(_pinout.dc, 0);

	reset_blocking();
	_submit_init_sequence();

	_dma_channel = dma_claim_unused_channel(true);
}

[[gnu::cold]] void DWO::init_dma_on_this_core() {
	dma_channel_config dma_cfg = dma_channel_get_default_config(_dma_channel);

	// TODO: is DMA_SIZE_32 (with transfer_count edited accordingly) supposed to
	// be scuffed?
	channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
	channel_config_set_dreq(&dma_cfg, spi_get_dreq(_spi, true));

	// read increment + no write increment is the default
	channel_config_set_read_increment(&dma_cfg, true);
	channel_config_set_write_increment(&dma_cfg, false);

	dma_channel_configure(_dma_channel, &dma_cfg, &spi_get_hw(_spi)->dr,
	                      nullptr, _scanline_r8g8b8.size(), false);

	// configure DMA IRQ and configure it to trigger when DMA scanned out a line
	dma_channel_set_irq0_enabled(_dma_channel, true);
	irq_set_exclusive_handler(DMA_IRQ_0, dwo_global_dma_handler);
	irq_set_enabled(DMA_IRQ_0, true);
}

[[gnu::cold]] void DWO::shutdown() {
	// TODO: cancel any in-flight IRQ, tear down/unclaim DMA, bring any GPIO to
	// defaults/unselected, etc.
}

[[gnu::cold]] void DWO::reset_blocking() {
	gpio_put(_pinout.pwr_en, 0);
	gpio_put(_pinout.rst, 0);
	sleep_ms(20);
	gpio_put(_pinout.rst, 1);
	sleep_ms(1);
	gpio_put(_pinout.rst, 0);
	sleep_ms(20);
	gpio_put(_pinout.rst, 1);
	sleep_ms(50);
	gpio_put(_pinout.pwr_en, 1);
}

[[gnu::cold]] void DWO::_submit_init_sequence() {
	write(Command::UNLOCK_CMD2, DataBuffer<1>{0x00});
	// TODO: document magic value
	write(Command::ENABLE_SPI_RAM_WRITE, DataBuffer<1>{0x80});
	write(Command::SET_PIXEL_FORMAT, DataBuffer<1>{0x77}); // 24bpp
	// TE{0x00} would be here
	write(Command::ENABLE_BRIGHTNESS_CTRL_BLOCK, DataBuffer<1>{0x20});
	set_brightness(0x00);
	write(Command::SET_HBM_BRIGHTNESS_FACTORY, DataBuffer<1>{0xFF});
	write(Command::SET_COLUMN, DataBuffer<4>{0x00, 0x16, 0x01, 0xAF});
	write(Command::SET_ROW, DataBuffer<4>{0x00, 0x00, 0x01, 0xF5});
	write(Command::DISABLE_SLEEP);
	sleep_ms(120);
	write(Command::DISPLAY_ON);
	set_brightness(0xFF);
}

void DWO::scanline_dma_update() {
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
		_current_dma_fb_offset + _scanline_r8g8b8.size() / 2;

	const devices::ScreenPalette screen_palette{_cloned_screen_palette};
	const devices::Framebuffer fb{_cloned_fb};

	// FIXME: FIX!!!!!

	_scanline_r8g8b8.fill(0xAA);

	/*for (std::size_t fb_idx = _current_dma_fb_offset, scanline_idx = 0;
	     fb_idx < current_fb_scanline_end; ++fb_idx, scanline_idx += 2) {
	    _scanline_r8g8b8[scanline_idx + 0] =
	        palette[screen_palette.get_color(fb.data[fb_idx] & 0x0F)];
	    _scanline_r8g8b8[scanline_idx + 1] =
	        palette[screen_palette.get_color(fb.data[fb_idx] >> 4)];
	}*/

	_current_dma_fb_offset = current_fb_scanline_end;

	// trigger new scanline transfer; we will get IRQ'd again when scaned out
	dma_channel_set_read_addr(_dma_channel, _scanline_r8g8b8.data(), true);
}

void DWO::start_scanout() {
	active_instance = this;

	if (_current_dma_fb_offset != 0) {
		// Scanout was still going on when we flipped, which will result in
		// tearing.
		// This is not really supposed to happen, but it isn't catastrophic.
		// Return from this; let it finish the frame.
		return;
	}

	write(Command::SET_COLUMN, DataBuffer<4>{x_start >> 8, x_start & 0xFF,
	                                         x_end >> 8, x_end & 0xFF});
	write(Command::SET_ROW, DataBuffer<4>{y_start >> 8, y_start & 0xFF,
	                                      y_end >> 8, y_end & 0xFF});

	// Start write
	write(Command::RAM_WRITE);

	// gpio_put(_pinout.dc, 1);
	gpio_put(_pinout.cs, 0);

	// trigger the first scanline write manually. IRQs will trigger the rest
	dwo_global_dma_handler();
}

DWO *DWO::active_instance;

} // namespace arch::pico::video
