#include "video/palette.hpp"
#include <hal/hal.hpp>

#include <cmdthread.hpp>
#include <hardwarestate.hpp>
#include <platform/picosystem/picosystem.hpp>

namespace hal {

namespace pico = arch::pico;
namespace picosystem = arch::pico::platform::picosystem;

std::uint16_t update_button_state() {
	std::uint16_t ret = 0;

	for (std::size_t i = 0; i < picosystem::hw.buttons.size(); ++i) {
		ret |= picosystem::hw.buttons[i] << i;
	}

	return ret;
}

void load_rgb_palette(std::span<std::uint32_t, 32> new_palette) {
	picosystem::hw.st7789.load_rgb_palette(new_palette);
}

std::span<const std::uint32_t, 32> get_default_palette() {
	return video::pico8_palette_rgb8;
}

} // namespace hal