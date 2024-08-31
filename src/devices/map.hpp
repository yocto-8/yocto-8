#pragma once

#include <emu/mmio.hpp>

namespace devices {

struct Map : emu::MMIODevice<8192> {
	using MMIODevice::MMIODevice;

	// NOTE: the first 4096 bytes overlap with the bottom of the sprite sheet
	static constexpr std::uint16_t default_map_address = 0x1000;

	static constexpr std::size_t width = 128, height = 128;

	static constexpr bool in_bounds(int x, int y) {
		return x >= 0 && y >= 0 && x < int(width) && y < int(height);
	}

	std::uint8_t &tile(std::uint8_t x, std::uint8_t y) const {
		// the 32 lower rows are stored at the _start_ of this memory area
		// TODO: if this is not optimized already by the compiler, this could be
		// done with a XOR
		std::uint8_t correct_y = (y >= 32) ? (y - 32) : (y + 32);

		return data[std::uintptr_t(x) + (std::uintptr_t(correct_y) * width)];
	}
};

} // namespace devices