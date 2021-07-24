#pragma once

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/dma.h>
#include <cstdint>
#include <cmath>
#include <emu/emulator.hpp>
#include <video/framebufferview.hpp>
#include <video/palette.hpp>
#include <gsl/span>

namespace arch::pico::video
{

static constexpr std::array<std::uint16_t, 32> rgb_palette_to_ssd1351_format(gsl::span<const std::uint32_t, 32> palette)
{
    std::array<std::uint16_t, 32> ret{};

    for (std::size_t i = 0; i < 32; ++i)
    {
        const std::uint32_t rgb8 = palette[i];

        std::uint8_t
            r5 = ((rgb8 >> 16) & 0xFF) >> (8 - 5),
            g6 = ((rgb8 >>  8) & 0xFF) >> (8 - 6),
            b5 = ((rgb8 >>  0) & 0xFF) >> (8 - 5);
            
        ret[i] = (r5 << (5 + 6)) | (g6 << 5) | b5;

        // Swap LSB/MSB
        ret[i] = ((ret[i] & 0xFF) << 8) | (ret[i] >> 8);
    }

    return ret;
}

class SSD1351
{
    public:
    enum class Command : std::uint8_t
    {
        SET_COLUMN = 0x15,
        SET_ROW = 0x75,

        RAM_WRITE = 0x5C,
        RAM_READ = 0x5D,

        SET_REMAP = 0xA0,
        SET_START_LINE = 0xA1,
        SET_DISPLAY_OFFSET = 0xA2,
        DISPLAY_ALL_OFF = 0xA4,
        DISPLAY_ALL_ON = 0xA5,

        NORMAL_DISPLAY = 0xA6,
        INVERT_DISPLAY = 0xA7,

        SET_FUNCTION = 0xAB,

        DISPLAY_OFF = 0xAE,
        DISPLAY_ON = 0xAF,

        SET_PRECHARGE_PERIOD = 0xB1,
        MAGIC_ENHANCE_DISPLAY = 0xB2, //< no idea what this does, the datasheet is not any more clear
        SET_CLOCK_DIVIDER = 0xB3,
        SET_VSL = 0xB4,
        SET_GPIO = 0xB5,
        SET_PRECHARGE2_PERIOD = 0xB6,

        SET_GRAYSCALE_LUT = 0xB8,
        SET_DEFAULT_LUT = 0xB9,

        SET_PRECHARGE_VOLTAGE = 0xBB,
        SET_COM_DESELECT_VOLTAGE = 0xBE,

        SET_CHANNEL_CONTRAST = 0xC1,
        SET_GLOBAL_CONTRAST = 0xC7,
        SET_MUX_RATIO = 0xCA,

        SET_COMMAND_LOCK = 0xFD,

        SET_HORIZONTAL_SCROLL = 0x96,

        STOP_HORIZONTAL_SCROLL = 0x9E,
        START_HORIZONTAL_SCROLL = 0x9F
    };

    struct Pinout
    {
        std::uint32_t sclk, tx, rst, cs, dc;
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
        gpio_put(_pinout.rst, 1);

        gpio_init(_pinout.cs);
        gpio_set_dir(_pinout.cs, GPIO_OUT);
        gpio_put(_pinout.cs, 1);

        gpio_init(_pinout.dc);
        gpio_set_dir(_pinout.dc, GPIO_OUT);
        gpio_put(_pinout.dc, 0);

        reset_blocking();
        submit_init_sequence();

        /*_dma_channel = dma_claim_unused_channel(true);
        dma_channel_config dma_cfg = dma_channel_get_default_config(_dma_channel);
        channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
        channel_config_set_dreq(&dma_cfg, spi_get_index(spi_default) ? DREQ_SPI1_TX : DREQ_SPI0_TX);*/
    }

    void reset_blocking()
    {
        gpio_put(_pinout.rst, 1);
        sleep_ms(1);
        gpio_put(_pinout.rst, 0);
        sleep_ms(1);
        gpio_put(_pinout.rst, 1);
        sleep_ms(1);
    }
    
    template<std::size_t N>
    using DataBuffer = std::array<std::uint8_t, N>;

    // scale 0x0..0xF
    void set_brightness(std::uint8_t scale)
    {
        write(Command::SET_GLOBAL_CONTRAST, DataBuffer<1>{scale});
    }

    void submit_init_sequence()
    {
        // Enable MCU interface (else some commands will be dropped)
        write(Command::SET_COMMAND_LOCK, DataBuffer<1>{0x12});
        write(Command::SET_COMMAND_LOCK, DataBuffer<1>{0xB1});

        // Shut off the display while we set it up
        write(Command::DISPLAY_OFF);

        // Set clock division stuff and cover the entire screen
        write(Command::SET_CLOCK_DIVIDER, DataBuffer<1>{0xF0});
        write(Command::SET_MUX_RATIO, DataBuffer<1>{127});

        // 64K 16-bit format, enable split, CBA, correct rotation
        write(Command::SET_REMAP, DataBuffer<1>{0b01110100});

        // Set display offsets
        write(Command::SET_START_LINE, DataBuffer<1>{0});
        write(Command::SET_DISPLAY_OFFSET, DataBuffer<1>{0});

        // Disable both GPIO pins
        write(Command::SET_GPIO, DataBuffer<1>{0x00});

        // Set SPI interface, disable sleep mode
        write(Command::SET_FUNCTION, DataBuffer<1>{0b00'1});

        // Timing tweaks
        write(Command::SET_PRECHARGE_PERIOD, DataBuffer<1>{0b0010'0010});
        write(Command::SET_PRECHARGE2_PERIOD, DataBuffer<1>{0b0001});

        // Voltage tweaks
        write(Command::SET_PRECHARGE_VOLTAGE, DataBuffer<1>{0b11111});
        write(Command::SET_COM_DESELECT_VOLTAGE, DataBuffer<1>{0b111});

        // Seems to be useless?
        //write(Command::MAGIC_ENHANCE_DISPLAY, DataBuffer<3>{0xA4, 0x00, 0x00});

        // Display regular pixel data
        write(Command::NORMAL_DISPLAY);

        // Contrast settings
        write(Command::SET_CHANNEL_CONTRAST, DataBuffer<3>{0xB9, 0xC2, 0xFF});
        //set_brightness(0xA); const float gamma = 1.25;
        set_brightness(0x7); const float gamma = 1.1;
        //set_brightness(0x2); const float gamma = 0.6;

        // Set cryptic command from the datasheet that does fuck knows
        write(Command::SET_VSL, DataBuffer<3>{0xA0, 0xB5, 0x55});

        DataBuffer<63> gamma_lut;
        for (std::size_t i = 0; i < gamma_lut.size(); ++i)
        {
            gamma_lut[i] = round(180.0f * pow(i/63.0f, gamma));
        }
        write(Command::SET_GRAYSCALE_LUT, gamma_lut);

        // Start display
        write(Command::DISPLAY_ON);
    }

    void write(Command command, gsl::span<const std::uint8_t> data = {})
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

    void update_frame(::video::FramebufferView::View view)
    {
        //const auto time_start = get_absolute_time();

        // SRAM writes should cover all the framebuffer (0..127)
        write(Command::SET_COLUMN, DataBuffer<2>{0, 127});
        write(Command::SET_ROW, DataBuffer<2>{0, 127});

        // Start write
        write(Command::RAM_WRITE);

        gpio_put(_pinout.dc, 1);
        gpio_put(_pinout.cs, 0);
        /*dma_channel_configure(
            _dma_channel,
            &dma_cfg,
            &spi_get_hw(_spi)->dr,
            emu::emulator.frame_buffer().data.data(),
            8192 / 4,
            false);*/

        for (std::size_t i = 0; i < ::video::FramebufferView::frame_bytes; ++i)
        {
            const auto pixel_pair = std::array{
                palette[view[i] & 0x0F],
                palette[view[i] >> 4]
            };
            
            spi_write_blocking(
                _spi,
                reinterpret_cast<const std::uint8_t*>(pixel_pair.data()),
                2 * pixel_pair.size()
            );
        }

        gpio_put(_pinout.cs, 1);

        //const auto time_end = get_absolute_time();
        //printf("%fms\n", absolute_time_diff_us(time_start, time_end) / 1000.0f);
    }

    static constexpr auto palette = rgb_palette_to_ssd1351_format(::video::pico8_palette_rgb8);

    private:
    //unsigned _dma_channel;
    spi_inst_t* _spi;
    Pinout _pinout;
};

}