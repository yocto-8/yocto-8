#include <emu/emulator.hpp>
#include <p8/parser.hpp>

#include <platform/platform.hpp>

namespace pico = arch::pico;

#include "cartridges/celeste.hpp"

int main()
{
    pico::platform::init_hardware();

    p8::Parser parser(cartridge);

    while (parser.parse_line())
        ;
    
    emu::emulator.run();

    /*for (;;)
    {
        {
            const auto time_start = get_absolute_time();
            emu::emulator.hook_update();
            const auto time_end = get_absolute_time();
            printf("_update() took %fms\n", absolute_time_diff_us(time_start, time_end) / 1000.0f);
        }

        {
            const auto time_start = get_absolute_time();
            pico::hw.ssd1351.update_frame();
            const auto time_end = get_absolute_time();
            printf("update_frame() took %fms\n", absolute_time_diff_us(time_start, time_end) / 1000.0f);
        }
    }*/
}