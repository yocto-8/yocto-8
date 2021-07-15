#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <video/ssd1351.hpp>
#include <io/button.hpp>

constexpr auto pico8_palette = video::driver::SSD1351::rgb_palette_to_native_format(std::array<std::uint32_t, 16>{
    0x000000, 0x1D2B53, 0x7E2553, 0x008751,
    0xAB5236, 0x5F574F, 0xC2C3C7, 0xFFF1E8,
    0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436,
    0x29ADFF, 0x83769C, 0xFF77A8, 0xFFCCAA
});

int main() {
    set_sys_clock_khz(250000, true);

    stdio_init_all();

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
    }
}