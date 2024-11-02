#pragma once

#include <cstdint>
#include <emu/mmio.hpp>

namespace devices {
struct ButtonState : emu::MMIODevice<8> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x5F4C;

	std::uint8_t &for_player(std::uint8_t player_id) const {
		return get_byte(player_id);
	}

	std::uint8_t is_pressed(std::uint8_t button_id,
	                        std::uint8_t player_id) const {
		return (for_player(player_id) >> button_id) & 0b1;
	}
};

} // namespace devices