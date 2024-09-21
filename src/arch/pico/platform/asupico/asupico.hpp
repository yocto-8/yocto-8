#pragma once

#include <io/pushbutton.hpp>
#include <video/ssd1351.hpp>

namespace arch::pico::platform::asupico {

struct HardwareState {
	video::SSD1351 ssd1351;
	std::array<io::PushButton, 6> buttons;
};

extern HardwareState hw;

void init_flash_frequency();
void init_default_frequency();
void init_stdio();
void init_buttons();
void init_emulator(std::size_t psram_size);
std::size_t __no_inline_not_in_flash_func(init_psram_pimoroni)();
void init_video_ssd1351();

} // namespace arch::pico::platform::asupico