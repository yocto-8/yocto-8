#include <emu/emulator.hpp>
#include <p8/parser.hpp>

#include "hardwarestate.hpp"

namespace pico = arch::pico;

#include "cartridges/oxo.hpp"

int main()
{
    pico::initialize_hardware();

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