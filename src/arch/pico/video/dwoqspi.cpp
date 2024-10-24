#include "dwoqspi.hpp"
#include "devices/image.hpp"
#include "devices/screenpalette.hpp"
#include <cstdio>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/spi.h>
#include <hardware/timer.h>

#include "dwoqspipio.pio.h"

namespace arch::pico::video {

uint32_t frame_start, vsync_last;

[[gnu::section(Y8_SRAM_SECTION), gnu::noinline]] void dwo_global_dma_handler() {
	DWO::active_instance->scanline_dma_update();
}

[[gnu::section(Y8_SRAM_SECTION), gnu::noinline]] void
dwo_vsync_signal_irq_handler(uint gpio, uint32_t events) {
	if (gpio == DWO::active_instance->_pinout.te) {
		const auto cur_time = time_us_64();

		// printf("%fms\n", (cur_time - vsync_last) / 1000.0f);
		vsync_last = cur_time;
		DWO::active_instance->start_scanout();
	}
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
	                         _pinout.sclk, clk_div);

	pio_sm_set_enabled(_pio, _pio_sm, true);
}

[[gnu::cold]] void DWO::init(Config config) {
	_spi = config.spi;
	_pio = config.pio;
	_pio_sm = config.pio_sm;
	_pinout = config.pinout;

	active_instance = this;

	_pio_offset = pio_add_program(_pio, &co5300_oled_program);
	release_assert(_pio_offset >= 0); // check no error
	co5300_oled_program_init(_pio, _pio_sm, _pio_offset, _pinout.sio0,
	                         _pinout.sclk, clk_div);

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

	gpio_set_irq_enabled_with_callback(_pinout.te, GPIO_IRQ_EDGE_RISE, true,
	                                   dwo_vsync_signal_irq_handler);
}

[[gnu::cold]] void DWO::init_dma_on_this_core() {
	dma_channel_config dma_cfg = dma_channel_get_default_config(_dma_channel);

	channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
	channel_config_set_dreq(&dma_cfg, pio_get_dreq(_pio, _pio_sm, true));
	channel_config_set_bswap(&dma_cfg, true);

	// read increment + no write increment is the default
	channel_config_set_read_increment(&dma_cfg, true);
	channel_config_set_write_increment(&dma_cfg, false);

	dma_channel_configure(_dma_channel, &dma_cfg, &_pio->txf[_pio_sm], nullptr,
	                      _ping_pong[0].size() / 4, false);

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
		SPI_WriteData((val >> 8) & 0xFF);                                      \
		SPI_WriteData((val >> 0) & 0xFF);                                      \
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
	write(Command::SET_PIXEL_FORMAT, DataBuffer<1>{0x55}); // 16bpp
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

	write(Command(0x31), DataBuffer<4>{x_start >> 8, x_start & 0xFF, x_end >> 8,
	                                   x_end & 0xFF});
	write(Command(0x30), DataBuffer<4>{y_start >> 8, y_start & 0xFF, y_end >> 8,
	                                   y_end & 0xFF});
	write(Command(0x12));
} // namespace arch::pico::video

void DWO::compute_scanline(std::span<std::uint8_t, 128 * 3 * 2> buffer,
                           std::size_t start_addr, std::size_t end_addr) {
	const devices::ScreenPalette screen_palette{_cloned_screen_palette};
	const devices::Framebuffer fb{_cloned_fb};

	for (std::size_t fb_idx = start_addr, scan_idx = 0; fb_idx < end_addr;
	     ++fb_idx, scan_idx += 3 * 2 * 2) {
		// bump scan_idx by:
		// repeated_pixels (3) * bytes per pixel (2) * pixels per fb byte
		// (2)

		const auto px1 =
			palette[screen_palette.get_color(fb.data[fb_idx] & 0x0F)];
		const auto px2 =
			palette[screen_palette.get_color(fb.data[fb_idx] >> 4)];

		// endianness is reversed -- why is that?

		for (std::size_t hor_repeat = 0; hor_repeat < 3; ++hor_repeat) {
			// px1.r, g, b
			buffer[scan_idx + hor_repeat * 2 + 0] = px1;
			buffer[scan_idx + hor_repeat * 2 + 1] = px1 >> 8;

			// px2.r, g, b
			buffer[scan_idx + 6 + hor_repeat * 2 + 0] = px2;
			buffer[scan_idx + 6 + hor_repeat * 2 + 1] = px2 >> 8;
		}
	}
}

[[gnu::section(Y8_SRAM_SECTION), gnu::hot]]
void DWO::scanline_dma_update() {
	// OK to call even if the function wasn't triggered by IRQ (i.e. on first
	// run)
	dma_channel_acknowledge_irq0(_dma_channel);

	if (_scanned_out_lines == rows) {
		// deselect chip and return
		_front_dma_fb_offset = 0;
		// printf("== %fms\n", (time_us_64() - frame_start) / 1000.0f);
		gpio_put(_pinout.cs, 1);
		return;
	}

	++_scanned_out_lines;

	// trigger line emit (there will be 3 of those)
	dma_channel_set_read_addr(_dma_channel, _back, true);

	if (_scanned_out_lines % 3 == 2) {
		// prepare buffer for next DREQ IRQ

		// write to front buffer
		unsigned front_dma_scanline_end = _front_dma_fb_offset + 128 / 2;
		compute_scanline(*_front, _front_dma_fb_offset, front_dma_scanline_end);
		_front_dma_fb_offset = front_dma_scanline_end;

		// swap front to back
		std::swap(_front, _back);
	}
}

[[gnu::section(Y8_SRAM_SECTION), gnu::hot]]
void DWO::start_scanout() {
	if ((_scanned_out_lines != 0 && _scanned_out_lines < rows) ||
	    !pio_sm_is_tx_fifo_empty(_pio, _pio_sm)) {
		// Scanout was still going on when we flipped, which will result in
		// tearing.
		// This is not really supposed to happen, but it isn't catastrophic.
		// Return from this; let it finish the frame.
		return;
	}

	frame_start = time_us_64();

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

	_front_dma_fb_offset = 0;
	_scanned_out_lines = 0;

	_front = &_ping_pong[0];
	_back = &_ping_pong[1];

	// prepare first back scanline for first DMA write
	compute_scanline(*_back, 0, 64);

	// trigger the first scanline write manually. IRQs will trigger the rest
	dwo_global_dma_handler();
}

DWO *DWO::active_instance;

} // namespace arch::pico::video
