#pragma once

#include "pico/types.h"
#include <array>

#include <video/ssd1351.hpp>
#include <io/pushbutton.hpp>
#include <pico/time.h>

namespace arch::pico
{

struct HardwareState
{
    video::SSD1351 ssd1351;
    std::array<io::PushButton, 6> buttons;
    absolute_time_t timer_start;
};

extern HardwareState hw;

void initialize_hardware();

}