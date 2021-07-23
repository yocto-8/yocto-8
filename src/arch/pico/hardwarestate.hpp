#pragma once

#include <array>

#include <video/ssd1351.hpp>
#include <io/pushbutton.hpp>

namespace arch::pico
{

struct HardwareState
{
    video::SSD1351 ssd1351;
    std::array<io::PushButton, 6> buttons;
};

extern HardwareState hw;

void initialize_hardware();

}