#include "dwoqspi.hpp"
#include "devices/image.hpp"
#include "devices/screenpalette.hpp"
#include <cstdio>
#include <hardware/dma.h>
#include <hardware/pio.h>
#include <hardware/spi.h>

#include "dwoqspipio.pio.h"

namespace arch::pico::video {

[[gnu::section(Y8_SRAM_SECTION), gnu::flatten, gnu::noinline]] void
dwo_global_dma_handler() {
	DWO::active_instance->scanline_dma_update();
}

void DWO::_switch_hardware_spi() {
	gpio_set_function(_pinout.sclk, GPIO_FUNC_SPI);
	gpio_set_function(_pinout.sio0, GPIO_FUNC_SPI);

	pio_sm_set_enabled(_pio, _pio_sm, false);
}

void DWO::_switch_pio() {
	gpio_init(_pinout.sclk);
	gpio_set_dir(_pinout.sclk, GPIO_OUT);
	gpio_put(_pinout.sclk, 0);

	gpio_init(_pinout.sio0);
	gpio_set_dir(_pinout.sio0, GPIO_OUT);
	gpio_put(_pinout.sio0, 0);

	gpio_set_function(_pinout.sclk, GPIO_FUNC_NULL);
	gpio_set_function(_pinout.sio0, GPIO_FUNC_NULL);

	co5300_oled_program_init(_pio, _pio_sm, _pio_offset, _pinout.sio0,
	                         _pinout.sclk, 3.5);

	pio_sm_set_enabled(_pio, _pio_sm, true);
}

[[gnu::cold]] void DWO::init(Config config) {
	_spi = config.spi;
	_pio = config.pio;
	_pio_sm = config.pio_sm;
	_pinout = config.pinout;
	_current_dma_fb_offset = 0;
	_current_line_repeat = 0;

	_pio_offset = pio_add_program(_pio, &co5300_oled_program);
	release_assert(_pio_offset >= 0); // check no error
	co5300_oled_program_init(_pio, _pio_sm, _pio_offset, _pinout.sio0,
	                         _pinout.sclk, 3.31);

	gpio_init(_pinout.te);
	gpio_set_dir(_pinout.te, GPIO_IN);
	gpio_put(_pinout.te, 1);

	gpio_init(_pinout.rst);
	gpio_set_dir(_pinout.rst, GPIO_OUT);
	gpio_put(_pinout.rst, 1);

	gpio_init(_pinout.cs);
	gpio_set_dir(_pinout.cs, GPIO_OUT);
	gpio_put(_pinout.cs, 1);

	gpio_init(_pinout.pwr_en);
	gpio_set_dir(_pinout.pwr_en, GPIO_OUT);
	gpio_put(_pinout.pwr_en, 0);

	_switch_hardware_spi();

	reset_blocking();
	_submit_init_sequence();

	_dma_channel = dma_claim_unused_channel(true);
}

[[gnu::cold]] void DWO::init_dma_on_this_core() {
	dma_channel_config dma_cfg = dma_channel_get_default_config(_dma_channel);

	channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
	channel_config_set_dreq(&dma_cfg, pio_get_dreq(_pio, _pio_sm, true));

	// read increment + no write increment is the default
	channel_config_set_read_increment(&dma_cfg, true);
	channel_config_set_write_increment(&dma_cfg, false);

	dma_channel_configure(_dma_channel, &dma_cfg, &_pio->txf[_pio_sm], nullptr,
	                      _scanline_r8g8b8.size() / 4, false);

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
	/*gpio_put(_pinout.pwr_en, 0);
	gpio_put(_pinout.rst, 0);
	sleep_ms(20);
	gpio_put(_pinout.rst, 1);
	sleep_ms(1);
	gpio_put(_pinout.rst, 0);
	sleep_ms(20);
	gpio_put(_pinout.rst, 1);
	sleep_ms(50);
	gpio_put(_pinout.pwr_en, 1);*/
}

[[gnu::cold]] void DWO::_submit_init_sequence() {
	/*
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
	write(Command(0x21));
	sleep_ms(120);
	write(Command::DISPLAY_ON);
	set_brightness(0xFF);*/

#define CLOCK_TIME 1 // us
#define RESET_H gpio_put(_pinout.rst, 1);
#define RESET_L gpio_put(_pinout.rst, 0);
#define HAL_Delay sleep_ms
#define SPI_CS_L                                                               \
	{ gpio_put(_pinout.cs, 0); }
#define SPI_CS_H                                                               \
	{                                                                          \
		gpio_put(_pinout.cs, 1);                                               \
		sleep_us(CLOCK_TIME);                                                  \
	}
#define SPI_1L_SendData(datx)                                                  \
	{                                                                          \
		uint8_t buf[1] = {uint8_t(datx)};                                      \
		spi_write_blocking(_spi, buf, 1);                                      \
	}
#define SPI_WriteComm(regval)                                                  \
	{                                                                          \
		SPI_1L_SendData(0x02);                                                 \
		SPI_1L_SendData(0x00);                                                 \
		SPI_1L_SendData(regval);                                               \
		SPI_1L_SendData(0x00);                                                 \
	}
#define SPI_WriteData(regval)                                                  \
	{ SPI_1L_SendData(regval); }
#define SPI_4wire_data_1wire_Addr(First_Byte, Addr)                            \
	{                                                                          \
		SPI_1L_SendData(First_Byte);                                           \
		SPI_1L_SendData(0x00);                                                 \
		SPI_1L_SendData(Addr);                                                 \
		SPI_1L_SendData(0x00);                                                 \
	}
#define SPI_4W_DATA_1W_ADDR_END()                                              \
	{                                                                          \
		SPI_CS_H;                                                              \
		SPI_CS_L;                                                              \
		SPI_4wire_data_1wire_Addr(0x32, 0x00);                                 \
		SPI_CS_H;                                                              \
	}
#define Write_Disp_Data(val)                                                   \
	{                                                                          \
		SPI_WriteData((val >> 16) & 0xFF);                                     \
		SPI_WriteData((val >> 8) & 0xFF);                                      \
		SPI_WriteData(val & 0xFF);                                             \
	}

	gpio_put(_pinout.pwr_en, 0);

	RESET_H; // RESET---HIGH
	HAL_Delay(1);
	RESET_L; // RESET---LOW
	HAL_Delay(10);
	RESET_H; // RESET---HIGH
	HAL_Delay(50);

	gpio_put(_pinout.pwr_en, 1);
	HAL_Delay(50);

	write(Command::UNLOCK_CMD2, DataBuffer<1>{0x00});
	write(Command::ENABLE_SPI_RAM_WRITE, DataBuffer<1>{0x80});
	write(Command::SET_PIXEL_FORMAT, DataBuffer<1>{0x77}); // 24bpp
	write(Command::SET_TE_MODE, DataBuffer<1>{0x00});
	write(Command::ENABLE_BRIGHTNESS_CTRL_BLOCK, DataBuffer<1>{0x20});
	set_brightness(0x00);
	write(Command::SET_HBM_BRIGHTNESS_FACTORY, DataBuffer<1>{0xFF});
	write(Command::DISABLE_SLEEP);
	sleep_ms(80);
	write(Command::DISPLAY_ON);
	set_brightness(0x90);

	// Clear whole buffer in SPI to black (just over SIO0)

#define COL 410
#define ROW 502

	unsigned Xstart = 0, Xend = COL - 1, Ystart = 0, Yend = ROW - 1;

	Xstart = Xstart + 0x16;
	Xend = Xend + 0x16;

	SPI_CS_L;
	SPI_WriteComm(0x2a); // Set Column Start Address
	SPI_WriteData(Xstart >> 8);
	SPI_WriteData(Xstart & 0xff);
	SPI_WriteData(Xend >> 8);
	SPI_WriteData(Xend & 0xff);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x2b); // Set Row Start Address
	SPI_WriteData(Ystart >> 8);
	SPI_WriteData(Ystart & 0xff);
	SPI_WriteData(Yend >> 8);
	SPI_WriteData(Yend & 0xff);
	SPI_CS_H;

	SPI_CS_L;
	SPI_4wire_data_1wire_Addr(0x02, 0x2C);
	for (int i = 0; i < ROW; ++i)
		for (int j = 0; j < COL; ++j) {
			Write_Disp_Data(0x000000);
		}
	SPI_CS_H;
} // namespace arch::pico::video

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

	unsigned current_fb_scanline_end = _current_dma_fb_offset + 128 / 2;

	const devices::ScreenPalette screen_palette{_cloned_screen_palette};
	const devices::Framebuffer fb{_cloned_fb};

	if (_current_line_repeat > 0) {

		for (std::size_t fb_idx = _current_dma_fb_offset, scan_idx = 0;
		     fb_idx < current_fb_scanline_end;
		     ++fb_idx, scan_idx += 3 * 3 * 2) {
			// bump scan_idx by:
			// repeated_pixels (3) * bytes per pixel (3) * pixels per fb byte
			// (2)

			const auto px1_rgb =
				palette[screen_palette.get_color(fb.data[fb_idx] & 0x0F)];
			const auto px2_rgb =
				palette[screen_palette.get_color(fb.data[fb_idx] >> 4)];

			// endianness is reversed -- why is that?

			for (std::size_t hor_repeat = 0; hor_repeat < 3; ++hor_repeat) {
				// px1.r, g, b
				_scanline_r8g8b8[scan_idx + hor_repeat * 3 + 0] = px1_rgb >> 16;
				_scanline_r8g8b8[scan_idx + hor_repeat * 3 + 1] = px1_rgb >> 8;
				_scanline_r8g8b8[scan_idx + hor_repeat * 3 + 2] = px1_rgb >> 0;

				// px2.r, g, b
				_scanline_r8g8b8[scan_idx + 9 + hor_repeat * 3 + 0] =
					px2_rgb >> 16;
				_scanline_r8g8b8[scan_idx + 9 + hor_repeat * 3 + 1] =
					px2_rgb >> 8;
				_scanline_r8g8b8[scan_idx + 9 + hor_repeat * 3 + 2] =
					px2_rgb >> 0;
			}
		}

		// swap endianness hack
		for (int i = 0; i < _scanline_r8g8b8.size(); i += 4) {
			std::swap(_scanline_r8g8b8[i], _scanline_r8g8b8[i + 3]);
			std::swap(_scanline_r8g8b8[i + 1], _scanline_r8g8b8[i + 2]);
		}
	}

	++_current_line_repeat;

	if (_current_line_repeat == 3) {
		_current_line_repeat = 0;
		// bump fb scanline
		_current_dma_fb_offset = current_fb_scanline_end;
	}

	// trigger new scanline transfer; we will get IRQ'd again when scaned
	// out
	dma_channel_set_read_addr(_dma_channel, _scanline_r8g8b8.data(), true);
}

void DWO::start_scanout() {
	active_instance = this;

	if (_current_dma_fb_offset != 0 || _current_line_repeat != 0 ||
	    !pio_sm_is_tx_fifo_empty(_pio, _pio_sm)) {
		// Scanout was still going on when we flipped, which will result in
		// tearing.
		// This is not really supposed to happen, but it isn't catastrophic.
		// Return from this; let it finish the frame.
		return;
	}

	// TODO: continuous write
	_switch_hardware_spi();

	write(Command::SET_COLUMN, DataBuffer<4>{x_start >> 8, x_start & 0xFF,
	                                         x_end >> 8, x_end & 0xFF});
	write(Command::SET_ROW, DataBuffer<4>{y_start >> 8, y_start & 0xFF,
	                                      y_end >> 8, y_end & 0xFF});

	gpio_put(_pinout.cs, 0);

	// QSPI write
	std::array<std::uint8_t, 4> qspi_cmd_buf = {
		0x32, 0x00, std::uint8_t(Command::RAM_WRITE), 0x00};

	spi_write_blocking(_spi, qspi_cmd_buf.data(), qspi_cmd_buf.size());

	_switch_pio();

	// trigger the first scanline write manually. IRQs will trigger the rest
	dwo_global_dma_handler();
}

DWO *DWO::active_instance;

} // namespace arch::pico::video
