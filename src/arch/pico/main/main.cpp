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

static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
i=0
function _update()
    c=i
    dir=1
    if btn(0) then dir=-1 end
    for y=0,127 do
        c=c+dir
        for x=0,127 do
            pset(x,y,(c+x)/16)
        end
	end
	i=i+1
end
__gfx__
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 
)";

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

/*
    const Button left(16), up(17), right(18), down(19), button_o(20), button_x(21);

    int off = 0, zoom = 1;
    std::uint8_t brightness = 0x8;

    for(;;)
    {
        display.set_brightness(brightness);

        ++off;

        if (left)
        {
            --off;
        }

        if (up)
        {
            ++brightness;
        }

        if (right)
        {
            ++off;
        }

        if (down)
        {
            --brightness;
        }

        if (button_o)
        {
            zoom += 1;
        }

        if (button_x)
        {
            zoom -= 1;
        }

        if (zoom < 1) { zoom = 1; }
        if (zoom > 16) { zoom = 16; }

        for (int y = 0; y < 128; ++y)
        {
            for (int x = 0; x < 128; ++x)
            {
                display.set_pixel(x, y, int((x + y + off) / (256*zoom / 16)) % 16);
            }
        }

        display.update_frame(pico8_palette);
    }*/
}