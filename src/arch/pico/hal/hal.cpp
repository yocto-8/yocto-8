#include "pico/time.h"
#include "video/palette.hpp"
#include <hal/hal.hpp>

#include <hardwarestate.hpp>
#include <cmdthread.hpp>

namespace hal
{

namespace pico = arch::pico;

// NOTE: some of the implementations are deferred to the platform.

void present_frame()
{
    pico::run_blocking_command(arch::pico::IoThreadCommand::PUSH_FRAME);
}

void reset_timer()
{
    pico::state.timer_start = get_absolute_time();
}

std::uint64_t measure_time_us()
{
    const auto current_time = get_absolute_time();
    return absolute_time_diff_us(pico::state.timer_start, current_time);
}

void delay_time_us(std::uint64_t time)
{
    sleep_us(time);
}

std::span<const std::uint32_t, 32> get_default_palette()
{
    return video::pico8_palette_rgb8;
}

}