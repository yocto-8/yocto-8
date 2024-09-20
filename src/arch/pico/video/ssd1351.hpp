#pragma once

#include <array>
#include <cstdint>
#include <hardware/dma.h>
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

	/// Configure a DMA channel to write 32-bit words to the SPI TX we use.
	/// This channel is later configured to read from the scanline buffer and
	/// call an IRQ after every scanline.
	void init_dma_on_this_core();

	void shutdown();

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
	[[gnu::always_inline]]
	void set_brightness(std::uint8_t scale) {
		write(Command::SET_GLOBAL_CONTRAST, DataBuffer<1>{scale});
	}

	[[gnu::always_inline]]
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

	void copy_framebuffer(devices::Framebuffer view,
	                      devices::ScreenPalette screen_palette) {
		view.clone_into(_cloned_fb);
		screen_palette.clone_into(_cloned_screen_palette);
	}

	void start_scanout();

	std::array<std::uint16_t, 32> palette;

	// HACK: we can't exactly have user data for the IRQ, but to be realistic,
	// with this DMA logic we long ago gave up on multi-device support
	static SSD1351 *active_instance;
	void scanline_dma_update();

	private:
	void _submit_init_sequence();

	std::array<std::uint16_t, 128> _scanline_buffer;
	unsigned _dma_channel;
	unsigned _current_dma_fb_offset;

	devices::Framebuffer::ClonedArray _cloned_fb;
	devices::ScreenPalette::ClonedArray _cloned_screen_palette;

	spi_inst_t *_spi;
	Pinout _pinout;
};

extern void ssd1351_global_dma_handler();

} // namespace arch::pico::video