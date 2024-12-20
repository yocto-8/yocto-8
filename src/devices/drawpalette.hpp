#pragma once

#include <cstdint>
#include <emu/mmio.hpp>

namespace devices {

struct DrawPalette : emu::MMIODevice<16> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x5F00;

	void reset() const {
		for (std::size_t i = 0; i < map_length; ++i) {
			set_color(i, i);
		}

		set_transparent(0);
	}

	void set_color(std::uint8_t palette_index,
	               std::uint8_t resolved_index) const {
		get_byte(palette_index) &= 0xF0;
		get_byte(palette_index) |= resolved_index & 0x0F;
	}

	std::uint8_t get_color(std::uint8_t palette_index) const {
		return get_byte(palette_index);
	}

	void set_transparent(std::uint8_t palette_index,
	                     bool transparent = true) const {
		get_byte(palette_index) &= 0x0F;
		get_byte(palette_index) |= int(transparent) << 4;
	}

	bool is_transparent(std::uint8_t palette_index) const {
		return (get_byte(palette_index) >> 4) != 0;
	}
};

} // namespace devices