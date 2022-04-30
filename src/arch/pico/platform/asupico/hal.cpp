#include <hal/hal.hpp>

#include <hardwarestate.hpp>
#include <cmdthread.hpp>
#include <platform/asupico/asupico.hpp>

namespace hal
{

namespace pico = arch::pico;
namespace asupico = arch::pico::platform::asupico;

std::uint16_t update_button_state()
{
    std::uint16_t ret = 0;

    for (std::size_t i = 0; i < asupico::hw.buttons.size(); ++i)
    {
        ret |= asupico::hw.buttons[i] << i;
    }

    return ret;
}

void load_rgb_palette(std::span<std::uint32_t, 32> new_palette)
{
    asupico::hw.ssd1351.load_rgb_palette(new_palette);
}

}