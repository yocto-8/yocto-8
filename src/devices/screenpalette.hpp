#pragma once

#include <emu/mmio.hpp>

namespace devices {

struct ScreenPalette : emu::MMIODevice<16> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x5F10;

	void reset() const {
		for (std::size_t i = 0; i < map_length; ++i) {
			set_raw_color(i, i);
		}
	}

	void set_raw_color(std::uint8_t palette_index,
	                   std::uint8_t resolved_index) const {
		get_byte(palette_index) = resolved_index;
	}

	std::uint8_t get_raw_color(std::uint8_t palette_index) const {
		return get_byte(palette_index);
	}

	std::uint8_t get_color(std::uint8_t palette_index) const {
		const auto raw = get_raw_color(palette_index);

		// if the 8th bit is set, the higher palette is selected.
		// right shift it to the 5th bit so that values 128..143 will
		// become 16..31. other bits loook like they should be ignored: mask
		// them out.
		return (raw & 0b0000'1111) | ((raw & 0b1000'0000) >> 3);
	}
};

} // namespace devices