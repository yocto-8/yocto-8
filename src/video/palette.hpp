#pragma once

#include <array>
#include <cstdint>

namespace video {

static constexpr std::array<std::uint32_t, 32> pico8_palette_rgb8{
	// 16 main colors
	0x000000, 0x1D2B53, 0x7E2553, 0x008751, 0xAB5236, 0x5F574F, 0xC2C3C7,
	0xFFF1E8, 0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436, 0x29ADFF, 0x83769C,
	0xFF77A8, 0xFFCCAA,

	// undocumented extra colors
	0x291814, 0x111D35, 0x422136, 0x125359, 0x742F29, 0x49333B, 0xA28879,
	0xF3EF7D, 0xBE1250, 0xFF6C24, 0xA8E72E, 0x00B543, 0x065AB5, 0x754665,
	0xFF6E59, 0xFF9D81};

constexpr std::uint32_t pack_rgb(std::uint8_t r, std::uint8_t g,
                                 std::uint8_t b) {
	return (r << 16) | (g << 8) | b;
}

static constexpr std::array<std::uint32_t, 32> ssd1351_precal_palette_rgb8{
	pack_rgb(0, 0, 0), pack_rgb(45, 60, 85), pack_rgb(100, 0, 80),
	pack_rgb(0, 121, 54), pack_rgb(149, 70, 45), pack_rgb(100, 120, 100),
	pack_rgb(150, 150, 180), pack_rgb(255, 230, 225), pack_rgb(255, 25, 90),
	pack_rgb(255, 148, 0), pack_rgb(255, 250, 17), pack_rgb(0, 255, 81),
	pack_rgb(70, 123, 255), pack_rgb(127, 88, 165), pack_rgb(238, 25, 159),
	pack_rgb(240, 179, 133),

	// undocumented extra colors, uncalibrated
	0x291814, 0x111D35, 0x422136, 0x125359, 0x742F29, 0x49333B, 0xA28879,
	0xF3EF7D, 0xBE1250, 0xFF6C24, 0xA8E72E, 0x00B543, 0x065AB5, 0x754665,
	0xFF6E59, 0xFF9D81};

} // namespace video