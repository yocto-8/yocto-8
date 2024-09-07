#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/time.h>
#include <span>

#include <devices/image.hpp>
#include <devices/screenpalette.hpp>
#include <emu/emulator.hpp>
#include <util/colors.hpp>

// std::pow isn't constexpr as of C++20
// let's just hardcode the table
// gamma = 1.25; values = [round(180*pow(i/63, gamma)) for i in range(63)];
// print(values)

constexpr std::array<std::uint8_t, 63> gamma_lut = {
	0,   1,   2,   4,   6,   8,   10,  12,  14,  16,  18,  20,  23,
	25,  27,  30,  32,  35,  38,  40,  43,  46,  48,  51,  54,  57,
	60,  62,  65,  68,  71,  74,  77,  80,  83,  86,  89,  93,  96,
	99,  102, 105, 108, 112, 115, 118, 121, 125, 128, 131, 135, 138,
	142, 145, 148, 152, 155, 159, 162, 166, 169, 173, 176};

namespace arch::pico::video {

class SSD1351 {
	public:
	enum class Command : std::uint8_t {
		SET_COLUMN = 0x15,
		SET_ROW = 0x75,

		RAM_WRITE = 0x5C,
		RAM_READ = 0x5D,

		SET_REMAP = 0xA0,
		SET_START_LINE = 0xA1,
		SET_DISPLAY_OFFSET = 0xA2,
		DISPLAY_ALL_OFF = 0xA4,
		DISPLAY_ALL_ON = 0xA5,

		NORMAL_DISPLAY = 0xA6,
		INVERT_DISPLAY = 0xA7,

		SET_FUNCTION = 0xAB,

		DISPLAY_OFF = 0xAE,
		DISPLAY_ON = 0xAF,

		SET_PRECHARGE_PERIOD = 0xB1,
		MAGIC_ENHANCE_DISPLAY = 0xB2, //< no idea what this does, the datasheet
		                              // is not any more clear
		SET_CLOCK_DIVIDER = 0xB3,
		SET_VSL = 0xB4,
		SET_GPIO = 0xB5,
		SET_PRECHARGE2_PERIOD = 0xB6,

		SET_GRAYSCALE_LUT = 0xB8,
		SET_DEFAULT_LUT = 0xB9,

		SET_PRECHARGE_VOLTAGE = 0xBB,
		SET_COM_DESELECT_VOLTAGE = 0xBE,

		SET_CHANNEL_CONTRAST = 0xC1,
		SET_GLOBAL_CONTRAST = 0xC7,
		SET_MUX_RATIO = 0xCA,

		SET_COMMAND_LOCK = 0xFD,

		SET_HORIZONTAL_SCROLL = 0x96,

		STOP_HORIZONTAL_SCROLL = 0x9E,
		START_HORIZONTAL_SCROLL = 0x9F
	};

	struct Pinout {
		std::uint32_t sclk, tx, rst, cs, dc;
	};

	struct Config {
		spi_inst_t *spi;
		Pinout pinout;
	};

	void init(Config config);

	void load_rgb_palette(std::span<const std::uint32_t, 32> new_rgb_palette) {
		palette = util::make_r5g6b5_palette(new_rgb_palette, true);
	}

	void reset_blocking() {
		gpio_put(_pinout.rst, 1);
		sleep_ms(1);
		gpio_put(_pinout.rst, 0);
		sleep_ms(1);
		gpio_put(_pinout.rst, 1);
		sleep_ms(1);
	}

	template <std::size_t N> using DataBuffer = std::array<std::uint8_t, N>;

	// scale 0x0..0xF
	void set_brightness(std::uint8_t scale) {
		write(Command::SET_GLOBAL_CONTRAST, DataBuffer<1>{scale});
	}

	void submit_init_sequence();

	inline void write(Command command,
	                  std::span<const std::uint8_t> data = {}) {
		gpio_put(_pinout.dc, 0);
		gpio_put(_pinout.cs, 0);
		spi_write_blocking(_spi, reinterpret_cast<std::uint8_t *>(&command), 1);

		if (!data.empty()) {
			gpio_put(_pinout.dc, 1);
			spi_write_blocking(_spi, data.data(), data.size());
		}
		gpio_put(_pinout.cs, 1);
	}

	[[gnu::flatten]]
	void
	__not_in_flash_func(update_frame)(devices::Framebuffer view,
	                                  devices::ScreenPalette screen_palette) {
		// const auto time_start = get_absolute_time();

		// SRAM writes should cover all the framebuffer (0..127)
		write(Command::SET_COLUMN, DataBuffer<2>{0, 127});
		write(Command::SET_ROW, DataBuffer<2>{0, 127});

		// Start write
		write(Command::RAM_WRITE);

		gpio_put(_pinout.dc, 1);
		gpio_put(_pinout.cs, 0);
		/*dma_channel_configure(
		    _dma_channel,
		    &dma_cfg,
		    &spi_get_hw(_spi)->dr,
		    emu::emulator.frame_buffer().data.data(),
		    8192 / 4,
		    false);*/

		for (std::size_t i = 0; i < view.frame_bytes; ++i) {
			const auto pixel_pair = std::array{
				palette[screen_palette.get_color(view.data[i] & 0x0F)],
				palette[screen_palette.get_color(view.data[i] >> 4)]};

			spi_write_blocking(
				_spi, reinterpret_cast<const std::uint8_t *>(pixel_pair.data()),
				2 * pixel_pair.size());
		}

		gpio_put(_pinout.cs, 1);

		// const auto time_end = get_absolute_time();
		// printf("%fms\n", absolute_time_diff_us(time_start, time_end) /
		// 1000.0f);
	}

	std::array<std::uint16_t, 32> palette;

	private:
	// unsigned _dma_channel;
	spi_inst_t *_spi;
	Pinout _pinout;
};

} // namespace arch::pico::video