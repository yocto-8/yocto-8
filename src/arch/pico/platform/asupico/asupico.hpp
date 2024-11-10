#pragma once

#include <io/pushbutton.hpp>
// #include <video/ssd1351.hpp>
#include <video/dwoqspi.hpp>

#include <ff.h>

namespace arch::pico::platform::asupico {

namespace state {
// video::SSD1351 ssd1351;
extern video::DWO dwo;
extern std::array<io::PushButton, 6> buttons;
extern FATFS flash_fatfs;
} // namespace state

void init_flash_frequency();
void init_default_frequency();
void init_stdio();
void init_basic_gpio();
void init_emulator(std::size_t psram_size);
std::size_t __no_inline_not_in_flash_func(init_psram_pimoroni)();
// void init_video_ssd1351();
void init_video_dwo();

} // namespace arch::pico::platform::asupico