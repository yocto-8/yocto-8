#pragma once

#include <cstdint>
#include <emu/mmio.hpp>

namespace devices {

struct ButtonState : emu::MMIODevice<2048> {
	using MMIODevice::MMIODevice;

	[[nodiscard]] auto custom_font() const { return sized_subspan<2048>(0); }
};

} // namespace devices