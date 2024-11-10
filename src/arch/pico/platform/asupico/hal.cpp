#include "video/palette.hpp"
#include <hal/hal.hpp>

#include <cmdthread.hpp>
#include <hardwarestate.hpp>
#include <platform/asupico/asupico.hpp>

namespace hal {

namespace pico = arch::pico;
namespace asupico = arch::pico::platform::asupico;

hal::ButtonState update_button_state() {
	std::uint8_t held_key_mask = 0;

	// FIXME: btnp implementation wrong and equivalent to btn

	for (std::size_t i = 0; i < asupico::state::buttons.size(); ++i) {
		held_key_mask |= asupico::state::buttons[i] << i;
	}

	return {.held_key_mask = held_key_mask, .pressed_key_mask = held_key_mask};
}

void load_rgb_palette(std::span<std::uint32_t, 32> new_palette) {
	asupico::state::dwo.load_rgb_palette(new_palette);
}

std::span<const std::uint32_t, 32> get_default_palette() {
	return video::pico8_palette_rgb8;
}

} // namespace hal