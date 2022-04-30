#include <platform/platform.hpp>
#include <platform/asupico/asupico.hpp>

#include <hardwarestate.hpp>
#include <video/palette.hpp>

namespace arch::pico::platform
{

void init_hardware()
{
    using namespace asupico;

    init_default_frequency();
    init_cmd_thread();
    init_stdio();
    init_buttons();
    init_spi_ram();
    init_video_ssd1351();
    init_emulator();
}

void present_frame(FrameCopiedCallback* callback)
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