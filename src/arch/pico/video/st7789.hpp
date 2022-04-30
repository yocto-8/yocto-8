/**

ST7789 is a display driver used in the PicoSystem.
Some code for this display driver has been taken from the PicoSystem SDK and
subsequently modified for yocto-8.

MIT License

Copyright (c) 2021 Pimoroni Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <cstdint>
#include <span>

#include <devices/image.hpp>
#include <devices/screenpalette.hpp>
#include <video/palette.hpp>
#include <util/colors.hpp>

namespace arch::pico::video
{

// TODO: deduplicate some common SPI logic with SSD1351
// TODO: abstract away some SPI logic so this is reusable across archs
// TODO: use DMA

class ST7789
{
    public:
    // TODO: properly document what these do
    enum class Command : std::uint8_t
    {
        SWRESET   = 0x01, TEON      = 0x35, MADCTL    = 0x36, COLMOD    = 0x3A,
        GCTRL     = 0xB7, VCOMS     = 0xBB, LCMCTRL   = 0xC0, VDVVRHEN  = 0xC2,
        VRHS      = 0xC3, VDVS      = 0xC4, FRCTRL2   = 0xC6, PWRCTRL1  = 0xD0,
        FRMCTR1   = 0xB1, FRMCTR2   = 0xB2, GMCTRP1   = 0xE0, GMCTRN1   = 0xE1,
        INVOFF    = 0x20, SLPOUT    = 0x11, DISPON    = 0x29, GAMSET    = 0x26,
        DISPOFF   = 0x28, RAMWR     = 0x2C, INVON     = 0x21, CASET     = 0x2A,
        RASET     = 0x2B
    };

    struct Pinout
    {
        std::uint32_t sclk, tx, cs, vsync, rst, dc, backlight;
    };

    struct Config
    {
        spi_inst_t* spi;
        Pinout pinout;
    };

    void init(Config config)
    {
        _spi = config.spi;
        _pinout = config.pinout;

        gpio_set_function(_pinout.sclk, GPIO_FUNC_SPI);
        gpio_set_function(_pinout.tx, GPIO_FUNC_SPI);

        gpio_init(_pinout.rst);
        gpio_set_dir(_pinout.rst, GPIO_OUT);
        //gpio_put(_pinout.rst, false);
        reset_blocking();

        gpio_init(_pinout.cs);
        gpio_set_dir(_pinout.cs, GPIO_OUT);
        gpio_put(_pinout.cs, true);

        gpio_init(_pinout.dc);
        gpio_set_dir(_pinout.dc, GPIO_OUT);
        gpio_put(_pinout.dc, false);

        gpio_init(_pinout.vsync);
        gpio_set_dir(_pinout.vsync, GPIO_IN);

        gpio_init(_pinout.backlight);
        gpio_set_dir(_pinout.backlight, GPIO_OUT);
        // HACK: force full brightness for now
        gpio_put(_pinout.backlight, true);

        submit_init_sequence();
    }

    void load_rgb_palette(std::span<const std::uint32_t, 32> new_rgb_palette)
    {
        palette = util::make_r5g6b5_palette(new_rgb_palette, true);
    }

    void reset_blocking()
    {
        gpio_put(_pinout.rst, false);
        sleep_ms(100); // TODO: attempt shrinking
        gpio_put(_pinout.rst, true);
    }
    
    template<std::size_t N>
    using DataBuffer = std::array<std::uint8_t, N>;

    void submit_init_sequence()
    {
        write(Command::SWRESET);
        sleep_ms(5);
        write(Command::MADCTL, DataBuffer<1>{0x04});
        write(Command::TEON, DataBuffer<1>{0x00});
        write(Command::FRMCTR2, DataBuffer<5>{0x0C, 0x0C, 0x00, 0x33, 0x33});
        write(Command::COLMOD, DataBuffer<1>{0b101});
        write(Command::GAMSET, DataBuffer<1>{0x01});
        write(Command::GCTRL, DataBuffer<1>{0x14});
        write(Command::VCOMS, DataBuffer<1>{0x25});
        write(Command::LCMCTRL, DataBuffer<1>{0x2C});
        write(Command::VDVVRHEN, DataBuffer<1>{0x01});
        write(Command::VRHS, DataBuffer<1>{0x12});
        write(Command::VDVS, DataBuffer<1>{0x20});
        write(Command::PWRCTRL1, DataBuffer<2>{0xA4, 0xA1});
        write(Command::FRCTRL2, DataBuffer<1>{0x1E});
        write(Command::GMCTRP1, DataBuffer<14>{
            0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F,
            0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23});
        write(Command::GMCTRN1, DataBuffer<14>{
            0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F,
            0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23});
        write(Command::INVON);
        sleep_ms(115);
        write(Command::SLPOUT);
        write(Command::DISPON);
        write(Command::CASET, DataBuffer<4>{0x00, 0x00, 0x00, 0xEF});
        write(Command::RASET, DataBuffer<4>{0x00, 0x00, 0x00, 0xEF});
        write(Command::RAMWR);
    }

    void write(Command command, std::span<const std::uint8_t> data = {})
    {
        gpio_put(_pinout.dc, 0);
        gpio_put(_pinout.cs, 0);
        spi_write_blocking(_spi, reinterpret_cast<std::uint8_t*>(&command), 1);

        if (!data.empty())
        {
            gpio_put(_pinout.dc, 1);
            spi_write_blocking(_spi, data.data(), data.size());
        }
        gpio_put(_pinout.cs, 1);
    }

    void update_frame(devices::Framebuffer view, devices::ScreenPalette screen_palette)
    {
        write(Command::CASET, DataBuffer<4>{0x00, 0x00, 0x00, 0xEF});
        write(Command::RASET, DataBuffer<4>{0x00, 0x00, 0x00, 0xEF});
        write(Command::RAMWR);

        gpio_put(_pinout.dc, 1);
        gpio_put(_pinout.cs, 0);

        // FIXME: this is only to see if anything shows up on screen at all
        //        we need the 240x240 cropping logic

        for (std::size_t fb_y = 0; fb_y < 120; fb_y += 1)
        {
            std::array<std::uint16_t, 240> scanline;
            const auto scanline_fb_byte_offset = (view.frame_width * fb_y) / 2;

            for (std::size_t fb_x = 0; fb_x < 120; fb_x += 2)
            {
                const auto byte_index = fb_x / 2 + scanline_fb_byte_offset;
                const auto pixel_pair = view.data[byte_index];

                scanline[fb_x * 2] = scanline[fb_x * 2 + 1] = palette[screen_palette.get_color(pixel_pair & 0x0F)];
                scanline[fb_x * 2 + 2] = scanline[fb_x * 2 + 3] = palette[screen_palette.get_color(pixel_pair >> 4)];
            }
            
            // double pixels vertically
            for (std::size_t i = 0; i < 2; ++i)
            {
                spi_write_blocking(
                    _spi,
                    reinterpret_cast<const std::uint8_t*>(scanline.data()),
                    scanline.size() * 2
                );
            }
        }

        gpio_put(_pinout.cs, 1);

        //const auto time_end = get_absolute_time();
        //printf("%fms\n", absolute_time_diff_us(time_start, time_end) / 1000.0f);
    }

    std::array<std::uint16_t, 32> palette;

    private:
    spi_inst_t* _spi;
    Pinout _pinout;
};

}