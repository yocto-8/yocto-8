#pragma once

#include <cstddef>
#include <cstdint>
#include <emu/mmio.hpp>

namespace devices {

struct Image : emu::MMIODevice<8192> {
	using MMIODevice::MMIODevice;

	static constexpr std::size_t frame_width = 128, frame_height = 128,
								 frame_pixels_per_byte = 2,
								 frame_bytes = (frame_width * frame_height) /
	                                           frame_pixels_per_byte;

	void clear(std::uint8_t palette_entry) const;

	void set_nibble(std::size_t i, std::uint8_t palette_entry) const;
	void set_pixel(std::uint8_t x, std::uint8_t y,
	               std::uint8_t palette_entry) const;

	std::uint8_t get_nibble(std::size_t i) const;
	std::uint8_t get_pixel(std::uint8_t x, std::uint8_t y) const;
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