#include "emu/emulator.hpp"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <extmem/spiram.hpp>
#include <extmem/cachedinterface.hpp>
#include <video/ssd1351.hpp>
#include <io/button.hpp>
#include <p8/parser.hpp>

#include "hardwarestate.hpp"

namespace pico = arch::pico;

// FIXME: __lua__ fucks up if a later chunk is not present

#include "cartridges/drippy.hpp"

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