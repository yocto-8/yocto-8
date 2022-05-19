#pragma once

#include <video/ssd1351.hpp>
#include <io/pushbutton.hpp>

namespace arch::pico::platform::asupico
{

struct HardwareState
{
    video::SSD1351 ssd1351;
    std::array<io::PushButton, 6> buttons;

    // we make this live statically rather than on the core1 stack because of stack size concerns
    devices::Framebuffer::ClonedArray fb_copy;
};

extern HardwareState hw;

void init_default_frequency();
void init_stdio();
void init_buttons();
void init_spi_ram();
void init_emulator();
void init_video_ssd1351();

}