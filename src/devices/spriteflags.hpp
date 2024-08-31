#pragma once

#include <emu/mmio.hpp>

namespace devices {

struct SpriteFlags : emu::MMIODevice<16> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x3000;

	[[nodiscard]] std::uint8_t &flags_for(std::uint8_t sprite_id) const {
		return data[sprite_id];
	}

	[[nodiscard]] bool get_flag(std::uint8_t sprite_id,
	                            std::uint8_t flag_index) const {
		return (flags_for(sprite_id) >> flag_index) & 0b1;
	}

	void set_flag(std::uint8_t sprite_id, std::uint8_t flag_index,
	              bool value) const {
		std::uint8_t &flags = flags_for(sprite_id);
		flags &= ~std::uint8_t(
			1 << flag_index); // create mask and reset bit at flag index
		flags |= std::uint8_t(value)
		         << flag_index; // OR bit at flag index (0) with value
	}
};

} // namespace devices