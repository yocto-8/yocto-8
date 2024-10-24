#include "dwoqspi.hpp"
#include "devices/image.hpp"
#include "devices/screenpalette.hpp"
#include <cstdio>
#include <hardware/dma.h>
#include <hardware/spi.h>

namespace arch::pico::video {

[[gnu::section(Y8_SRAM_SECTION), gnu::flatten, gnu::noinline]] void
dwo_global_dma_handler() {
	DWO::active_instance->write(
		DWO::Command::SET_COLUMN,
		DWO::DataBuffer<4>{DWO::x_start >> 8, DWO::x_start & 0xFF,
	                       DWO::x_end >> 8, DWO::x_end & 0xFF});
	DWO::active_instance->write(
		DWO::Command::SET_ROW,
		DWO::DataBuffer<4>{DWO::y_start >> 8, DWO::y_start & 0xFF,
	                       DWO::y_end >> 8, DWO::y_end & 0xFF});
	DWO::active_instance->write(DWO::Command::RAM_WRITE);
	DWO::active_instance->scanline_dma_update();
}

[[gnu::cold]] void DWO::init(Config config) {
	_spi = config.spi;
	_pinout = config.pinout;
	_current_dma_fb_offset = 0;

	// gpio_set_function(_pinout.sclk, GPIO_FUNC_SPI);
	// gpio_set_function(_pinout.sio0, GPIO_FUNC_SPI);

	gpio_init(_pinout.sclk);
	gpio_set_dir(_pinout.sclk, GPIO_OUT);
	gpio_put(_pinout.sclk, 1);

	gpio_init(_pinout.sio0);
	gpio_set_dir(_pinout.sio0, GPIO_OUT);
	gpio_put(_pinout.sio0, 1);

	gpio_init(_pinout.rst);
	gpio_set_dir(_pinout.rst, GPIO_OUT);
	gpio_put(_pinout.rst, 1);

	gpio_init(_pinout.cs);
	gpio_set_dir(_pinout.cs, GPIO_OUT);
	gpio_put(_pinout.cs, 1);

	gpio_init(_pinout.pwr_en);
	gpio_set_dir(_pinout.pwr_en, GPIO_OUT);
	gpio_put(_pinout.pwr_en, 0);

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
	/*write(Command::UNLOCK_CMD2, DataBuffer<1>{0x00});
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
#define SPI_SDA_H gpio_put(_pinout.sio0, 1)
#define SPI_SDA_L gpio_put(_pinout.sio0, 0)
#define SPI_CLK_H                                                              \
	{ gpio_put(_pinout.sclk, 1); }
#define SPI_CLK_L                                                              \
	{                                                                          \
		sleep_us(CLOCK_TIME);                                                  \
		gpio_put(_pinout.sclk, 0);                                             \
		sleep_us(CLOCK_TIME);                                                  \
	}
#define SPI_1L_SendData(datx)                                                  \
	{                                                                          \
		unsigned char dat = datx;                                              \
		unsigned char i;                                                       \
		for (i = 0; i < 8; i++) {                                              \
			if ((dat & 0x80) != 0) {                                           \
				SPI_SDA_H;                                                     \
			} else {                                                           \
				SPI_SDA_L;                                                     \
			}                                                                  \
			dat <<= 1;                                                         \
			SPI_CLK_L;                                                         \
			SPI_CLK_H;                                                         \
		}                                                                      \
	}
#define SPI_1L_ReadData(xptr)                                                  \
	{                                                                          \
		*xptr = 0;                                                             \
		unsigned char i;                                                       \
		for (i = 0; i < 8; i++) {                                              \
			SPI_CLK_L;                                                         \
			SPI_CLK_H;                                                         \
			*xptr <<= 1;                                                       \
			*xptr |= gpio_get(_pinout.sio0);                                   \
		}                                                                      \
	}
#define SPI_WriteComm(regval)                                                  \
	{                                                                          \
		SPI_1L_SendData(0x02);                                                 \
		SPI_1L_SendData(0x00);                                                 \
		SPI_1L_SendData(regval);                                               \
		SPI_1L_SendData(0x00);                                                 \
	}
#define SPI_ReadComm(regval, count)                                            \
	{                                                                          \
		SPI_1L_SendData(0x03);                                                 \
		SPI_1L_SendData(0x00);                                                 \
		SPI_1L_SendData(regval);                                               \
		SPI_1L_SendData(0x00);                                                 \
		gpio_set_dir(_pinout.sio0, GPIO_IN);                                   \
		for (int i = 0; i < count; ++i) {                                      \
			unsigned char x;                                                   \
			SPI_1L_ReadData(&x);                                               \
			printf("%02X ", x);                                                \
		}                                                                      \
		gpio_set_dir(_pinout.sio0, GPIO_OUT);                                  \
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
#define QSPI_WriteData(x)                                                      \
	{                                                                          \
		SPI_WriteData((rand() % 2) ? 0xFF : 0x00);                             \
		SPI_WriteData(0x00);                                                   \
		SPI_WriteData(0x00);                                                   \
	}
#define Write_Disp_Data(val)                                                   \
	{                                                                          \
		QSPI_WriteData(val >> 8);                                              \
		QSPI_WriteData(val);                                                   \
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

	/*for (int i = 0; i < 2; ++i) {
	    printf("ATTEMPTING CHIP IDENT: ");
	    SPI_CS_L;
	    SPI_ReadComm(0x04, 3);
	    SPI_CS_H;
	    printf("\n");
	}

	for (int i = 0; i < 2; ++i) {
	    printf("READ DISPLAY BRIGHTNESS: ");
	    SPI_CS_L;
	    SPI_ReadComm(0x52, 1);
	    SPI_CS_H;
	    printf("\n");
	}

	for (int i = 0; i < 2; ++i) {
	    printf("READ HBM DISPLAY BRIGHTNESS: ");
	    SPI_CS_L;
	    SPI_ReadComm(0x64, 1);
	    SPI_CS_H;
	    printf("\n");
	}

	for (int i = 0; i < 2; ++i) {
	    printf("READ CTRL DISPLAY: ");
	    SPI_CS_L;
	    SPI_ReadComm(0x52, 1);
	    SPI_CS_H;
	    printf("\n");
	}*/

	SPI_CS_L;
	SPI_WriteComm(0xFE);
	SPI_WriteData(0x00);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0xC4); // QSPI setting, MIPI remove
	SPI_WriteData(0x80);
	SPI_CS_H;

	// for (int i = 0; i < 2; ++i) {
	// 	printf("ATTEMPTING CHIP IDENT: ");
	// 	SPI_CS_L;
	// 	SPI_ReadComm(0x04, 3);
	// 	SPI_CS_H;
	// 	printf("\n");
	// }

	// for (int i = 0; i < 2; ++i) {
	// 	printf("READ DISPLAY BRIGHTNESS: ");
	// 	SPI_CS_L;
	// 	SPI_ReadComm(0x52, 1);
	// 	SPI_CS_H;
	// 	printf("\n");
	// }

	// for (int i = 0; i < 2; ++i) {
	// 	printf("READ HBM DISPLAY BRIGHTNESS: ");
	// 	SPI_CS_L;
	// 	SPI_ReadComm(0x64, 1);
	// 	SPI_CS_H;
	// 	printf("\n");
	// }

	// for (int i = 0; i < 2; ++i) {
	// 	printf("READ CTRL DISPLAY: ");
	// 	SPI_CS_L;
	// 	SPI_ReadComm(0x52, 1);
	// 	SPI_CS_H;
	// 	printf("\n");
	// }

	SPI_CS_L;
	SPI_WriteComm(0x3A);
	// SPI_WriteData(0x55); // Interface Pixel Format	16bit/pixel
	// SPI_WriteData(0x66); //Interface Pixel Format	18bit/pixel
	SPI_WriteData(0x77); // Interface Pixel Format	24bit/pixel
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x35); // TE ON
	SPI_WriteData(0x00);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x53);
	SPI_WriteData(0x20);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x51); // Write Display Brightness	MAX_VAL=0XFF
	SPI_WriteData(0x00);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x63);
	SPI_WriteData(0xFF);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x2A);
	SPI_WriteData(0x00);
	SPI_WriteData(0x16);
	SPI_WriteData(0x01);
	SPI_WriteData(0xAF);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x2B);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x01);
	SPI_WriteData(0xF5);
	SPI_CS_H;

	SPI_CS_L;
	SPI_WriteComm(0x11); // Sleep out
	SPI_CS_H;
	HAL_Delay(80);

	SPI_CS_L;
	SPI_WriteComm(0x29); // Display on
	SPI_CS_H;
	HAL_Delay(20);

	SPI_CS_L;
	SPI_WriteComm(0x51); // Write Display Brightness	MAX_VAL=0XFF
	SPI_WriteData(0xFF);
	SPI_CS_H;

	for (int i = 0; i < 2; ++i) {
		printf("ATTEMPTING CHIP IDENT: ");
		SPI_CS_L;
		SPI_ReadComm(0x04, 3);
		SPI_CS_H;
		printf("\n");
	}

	for (int i = 0; i < 2; ++i) {
		printf("READ DISPLAY BRIGHTNESS: ");
		SPI_CS_L;
		SPI_ReadComm(0x52, 1);
		SPI_CS_H;
		printf("\n");
	}

	for (int i = 0; i < 2; ++i) {
		printf("READ HBM DISPLAY BRIGHTNESS: ");
		SPI_CS_L;
		SPI_ReadComm(0x64, 1);
		SPI_CS_H;
		printf("\n");
	}

	for (int i = 0; i < 2; ++i) {
		printf("READ CTRL DISPLAY: ");
		SPI_CS_L;
		SPI_ReadComm(0x52, 1);
		SPI_CS_H;
		printf("\n");
	}

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
	SPI_WriteComm(0x2c); // Memory Write
	SPI_CS_H;

	unsigned color = 0xF800;

	// SPI_4W_DATA_1W_ADDR_START();
	SPI_CS_L;
	SPI_4wire_data_1wire_Addr(0x02, 0x2C);

	for (int i = 0; i < ROW; i++)
		for (int j = 0; j < COL; j++) {
			Write_Disp_Data(color);
		}
	SPI_CS_H;
	// SPI_4W_DATA_1W_ADDR_END();
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
	/*active_instance = this;

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
	// write(Command::RAM_WRITE);

	gpio_put(_pinout.cs, 0);

	std::array<std::uint8_t, 4> qspi_cmd_buf = {
	    0x02, 0x00, std::uint8_t(Command::RAM_WRITE), 0x00};

	spi_write_blocking(_spi, qspi_cmd_buf.data(), qspi_cmd_buf.size());

	// trigger the first scanline write manually. IRQs will trigger the rest
	dwo_global_dma_handler()*/
}

DWO *DWO::active_instance;

} // namespace arch::pico::video
