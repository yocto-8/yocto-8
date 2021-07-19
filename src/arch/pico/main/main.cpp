#include "emu/emulator.hpp"
#include "hardware/gpio.h"
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

constexpr auto pico8_palette = video::driver::SSD1351::rgb_palette_to_native_format(std::array<std::uint32_t, 16>{
    0x000000, 0x1D2B53, 0x7E2553, 0x008751,
    0xAB5236, 0x5F574F/*0x6F675F*/, 0xC2C3C7, 0xFFF1E8,
    0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436,
    0x29ADFF, 0x83769C, 0xFF77A8, 0xFFCCAA
});

static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
i=0
function _update()
    c=i
	for y=0,127 do
		c=c+1
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

int main() {
    set_sys_clock_khz(250000, true);

    stdio_init_all();
    
    extmem::spiram::setup();

    /*std::array<char, 1024 * 48> yolo;
    emu::emulator.init(gsl::span<char, 1024*48>(
        reinterpret_cast<char*>(yolo.data()),
        reinterpret_cast<char*>(yolo.data() + yolo.size())
    ));*/
    
    emu::emulator.init(gsl::span<char, extmem::bank_size>(
        reinterpret_cast<char*>(extmem::bank_base),
        reinterpret_cast<char*>(extmem::bank_base + extmem::bank_size)
    ));

    spi_inst_t* spi_port = spi0;

    // 25MHz - slightly out of spec but 20MHz is not achievable as a clock division from 250MHz, AFAICT
    spi_init(spi_port, 25 * 1000 * 1000);

    video::driver::SSD1351::Config config;
    config.spi = spi_port;
    config.pinout.sclk = 2;
    config.pinout.tx = 3;
    config.pinout.rst = 4;
    config.pinout.cs = 5;
    config.pinout.dc = 6;
    video::driver::SSD1351 display(config);

    p8::Parser parser(cartridge);

    while (parser.parse_line())
        ;

    for (;;)
    {
        const auto time_start = get_absolute_time();
        emu::emulator.hook_update();
        const auto time_end = get_absolute_time();
        printf("_update() took %fms\n", absolute_time_diff_us(time_start, time_end) / 1000.0f);

        display.update_frame(pico8_palette);
    }

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