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
		fill(pixel_pair_byte);
	}

	void set_pixel(std::uint8_t x, std::uint8_t y,
	               std::uint8_t palette_entry) const {
		set_nibble(x + (y * frame_width), palette_entry,
		           emu::NibbleOrder::LSB_FIRST);
	}

	std::uint8_t get_pixel(std::uint8_t x, std::uint8_t y) const {
		return get_nibble(x + (y * frame_width), emu::NibbleOrder::LSB_FIRST);
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