#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <emu/mmio.hpp>

namespace devices {

struct Image : emu::MMIODevice<8192> {
	using MMIODevice::MMIODevice;

	static constexpr std::size_t frame_width = 128, frame_height = 128,
								 frame_pixels_per_byte = 2,
								 frame_bytes = (frame_width * frame_height) /
	                                           frame_pixels_per_byte;

	void clear(std::uint8_t palette_entry) const {
		std::uint8_t pixel_pair_byte = palette_entry | (palette_entry << 4);
		std::memset(data.data(), pixel_pair_byte, data.size());
	}

	void set_nibble(std::size_t i, std::uint8_t palette_entry) const {
		std::uint8_t &pixel_pair_byte = data[i / 2];

		if (i % 2 == 0) // lower pixel?
		{
			pixel_pair_byte &= 0xF0;                 // clear lower nibble
			pixel_pair_byte |= palette_entry & 0x0F; // set lower nibble
		} else {
			pixel_pair_byte &= 0x0F;               // clear upper nibble
			pixel_pair_byte |= palette_entry << 4; // set upper nibble
		}
	}

	void set_pixel(std::uint8_t x, std::uint8_t y,
	               std::uint8_t palette_entry) const {
		set_nibble(x + (y * frame_width), palette_entry);
	}

	std::uint8_t get_nibble(std::size_t i) const {
		std::uint8_t pixel_pair_byte = data[i / 2];

		if (i % 2 == 0) // lower pixel?
		{
			return pixel_pair_byte & 0x0F;
		}

		return pixel_pair_byte >> 4;
	}
	std::uint8_t get_pixel(std::uint8_t x, std::uint8_t y) const {
		return get_nibble(x + (y * frame_width));
	}
};

struct Framebuffer : Image {
	using Image::Image;
	static constexpr auto default_map_address = 0x6000;
};

struct Spritesheet : Image {
	using Image::Image;
	static constexpr auto default_map_address = 0x0000;
};

} // namespace devices