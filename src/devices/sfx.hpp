#pragma once

#include <emu/mmio.hpp>

namespace devices {

struct Sfx : emu::MMIODevice<256> {
	using MMIODevice::MMIODevice;

	static constexpr y8::PicoAddr default_map_address = 0x3200;
};

} // namespace devices
