#include <hal/hal.hpp>

#include <hardwarestate.hpp>

namespace hal
{

std::uint16_t update_button_state()
{
    std::uint16_t ret = 0;

    for (std::size_t i = 0; i < arch::pico::hw.buttons.size(); ++i)
    {
        ret |= arch::pico::hw.buttons[i] << i;
    }

    return ret;
}

void present_frame(video::Framebuffer::View view)
{
    // TODO: double-buffering
    arch::pico::hw.ssd1351.update_frame(view);
}

}