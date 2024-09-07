#include "ssd1351.hpp"

[[gnu::cold]] void arch::pico::video::SSD1351::init(Config config) {
	_spi = config.spi;
	_pinout = config.pinout;

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
	submit_init_sequence();

	/*_dma_channel = dma_claim_unused_channel(true);
	dma_channel_config dma_cfg =
	dma_channel_get_default_config(_dma_channel);
	channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
	channel_config_set_dreq(&dma_cfg, spi_get_index(spi_default) ?
	DREQ_SPI1_TX : DREQ_SPI0_TX);*/
}

[[gnu::cold]] void arch::pico::video::SSD1351::submit_init_sequence() {
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
