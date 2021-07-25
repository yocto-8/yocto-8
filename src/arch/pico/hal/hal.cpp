#include "pico/time.h"
#include <hal/hal.hpp>

#include <hardwarestate.hpp>
#include <cmdthread.hpp>

namespace hal
{

namespace pico = arch::pico;

std::uint16_t update_button_state()
{
    std::uint16_t ret = 0;

    for (std::size_t i = 0; i < pico::hw.buttons.size(); ++i)
    {
        ret |= pico::hw.buttons[i] << i;
    }

    return ret;
}

void present_frame()
{
    // TODO: double-buffering
    pico::run_blocking_command(arch::pico::IoThreadCommand::PUSH_FRAME);
    //arch::pico::hw.ssd1351.update_frame(view.data);
}

void reset_timer()
{
    pico::hw.timer_start = get_absolute_time();
}

std::uint64_t measure_time_us()
{
    const auto current_time = get_absolute_time();
    return absolute_time_diff_us(pico::hw.timer_start, current_time);
}

}