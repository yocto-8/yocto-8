#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <emu/mmio.hpp>

namespace devices {

struct Music : emu::MMIODevice<256> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x3100;
};

} // namespace devices
