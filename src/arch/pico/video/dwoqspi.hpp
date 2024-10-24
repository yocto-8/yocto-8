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

namespace arch::pico::video {

class DWO {
	public:
	enum class Command : std::uint8_t {
		UNLOCK_CMD2 = 0xFE,
		ENABLE_SPI_RAM_WRITE = 0xC4, // TODO: what are the values, is QSPI
		                             // setting working for 3-wire SPI
		SET_PIXEL_FORMAT = 0x3A,
		// SET_TE_MODE = 0x35,
		ENABLE_BRIGHTNESS_CTRL_BLOCK = 0x53,
		SET_BRIGHTNESS = 0x51,
		/// Possibly the factory configuration for brightness?
		SET_HBM_BRIGHTNESS_FACTORY = 0x63,
		DISABLE_SLEEP = 0x11,
		DISPLAY_ON = 0x29,
		SET_COLUMN = 0x2A,
		SET_ROW = 0x2B,
		RAM_WRITE = 0x2C,
		RAM_WRITE_CONTINUOUS = 0x3C,
	};

	static constexpr std::size_t real_columns = 410, real_rows = 502;
	static constexpr std::size_t columns = 128 * 3, rows = 128 * 3;

	// TODO: document why is the 0x16 offset required?
	static constexpr std::size_t x_start = (real_columns - columns) / 2 + 0x16,
								 y_start = (real_rows - rows) / 2;
	// inclusive
	static constexpr std::size_t x_end = x_start + columns - 1,
								 y_end = y_start + rows - 1;

	struct Pinout {
		// Using """QSPI""", but only sio0 is driven

		std::uint32_t sclk, cs;
		// QSPI pins; sio0 is also the regular SPI I/O pin
		std::uint32_t sio0, qsi1, qsi2, qsi3;
		/// Tearing Enable (input, basically vsync, optional, float if unused)
		// std::uint32_t te;
		///
		std::uint32_t rst, pwr_en;
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
		std::memcpy(palette.data(), new_rgb_palette.data(),
		            new_rgb_palette.size_bytes());
	}

	void reset_blocking();

	template <std::size_t N> using DataBuffer = std::array<std::uint8_t, N>;

	// scale 0x0..0xFF
	void set_brightness(std::uint8_t scale) {
		write(Command::SET_BRIGHTNESS, DataBuffer<1>{scale});
	}

	inline void write(Command command, std::span<const std::uint8_t> data = {},
	                  bool release_cs = true) {
		// gpio_put(_pinout.dc, 0);
		gpio_put(_pinout.cs, 0);

		std::array<std::uint8_t, 4> qspi_cmd_buf = {
			0x02, 0x00, std::uint8_t(command), 0x00};

		spi_write_blocking(_spi, qspi_cmd_buf.data(), qspi_cmd_buf.size());

		if (!data.empty()) {
			// gpio_put(_pinout.dc, 1);
			spi_write_blocking(_spi, data.data(), data.size());
		}

		if (release_cs) {
			gpio_put(_pinout.cs, 1);
		}

		// printf("write( %02X args ", int(command));
		// for (auto c : data)
		// 	printf("%02X ", c);
		// printf(")\n");
	}

	void copy_framebuffer(devices::Framebuffer view,
	                      devices::ScreenPalette screen_palette) {
		view.clone_into(_cloned_fb);
		screen_palette.clone_into(_cloned_screen_palette);
	}

	void start_scanout();

	std::array<std::uint32_t, 32> palette;

	// HACK: we can't exactly have user data for the IRQ, but to be realistic,
	// with this DMA logic we long ago gave up on multi-device support
	static DWO *active_instance;
	void scanline_dma_update();

	private:
	void _submit_init_sequence();

	std::array<std::uint8_t, 128 * 3 * 3> _scanline_r8g8b8;
	unsigned _dma_channel;
	unsigned _current_dma_fb_offset;
	unsigned _current_line_repeat;

	devices::Framebuffer::ClonedArray _cloned_fb;
	devices::ScreenPalette::ClonedArray _cloned_screen_palette;

	spi_inst_t *_spi;
	Pinout _pinout;
};

extern void dwo_global_dma_handler();

} // namespace arch::pico::video