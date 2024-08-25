#include <cstdio>
#include <platform/platform.hpp>
#include <platform/asupico/asupico.hpp>

#include <hardwarestate.hpp>
#include <video/palette.hpp>

namespace arch::pico::platform
{

void init_hardware()
{
    using namespace asupico;

    printf("Configuring core frequency\n");
    init_default_frequency();
    printf("Booting command thread\n");
    init_cmd_thread();
    printf("Configuring stdio\n");
    init_stdio();
    printf("Configuring buttons\n");
    init_buttons();
    // printf("Configuring SPI RAM\n");
    // init_spi_ram();
    printf("Configuring video\n");
    init_video_ssd1351();
    printf("Configuring emulator\n");
    init_emulator();
    printf("Hardware init done\n");
}

void __not_in_flash_func(present_frame)(FrameCopiedCallback* callback)
{
    emu::device<devices::Framebuffer>.clone_into(asupico::hw.fb_copy);

    devices::ScreenPalette::ClonedArray palette_copy;
    emu::device<devices::ScreenPalette>.clone_into(palette_copy);

    if (callback != nullptr)
    {
        callback();
    }

    asupico::hw.ssd1351.update_frame(
        devices::Framebuffer{std::span(asupico::hw.fb_copy)},
        devices::ScreenPalette{std::span(palette_copy)}
    );
}

std::span<const std::uint32_t, 32> get_default_palette()
{
    return ::video::ssd1351_precal_palette_rgb8;
}

}