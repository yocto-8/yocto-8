#include "video/palette.hpp"
#include <platform/platform.hpp>
#include <platform/picosystem/picosystem.hpp>

#include <hardwarestate.hpp>
#include <emu/emulator.hpp>
#include <devices/image.hpp>

namespace arch::pico::platform
{

void init_hardware()
{
    using namespace picosystem;

    //init_default_frequency();
    init_cmd_thread();
    init_stdio();
    init_buttons();
    init_video_st7789();
    init_emulator();
}

void present_frame(FrameCopiedCallback* callback)
{
    emu::device<devices::Framebuffer>.clone_into(picosystem::hw.fb_copy);

    devices::ScreenPalette::ClonedArray palette_copy;
    emu::device<devices::ScreenPalette>.clone_into(palette_copy);

    if (callback != nullptr)
    {
        callback();
    }

    picosystem::hw.st7789.update_frame(
        devices::Framebuffer{std::span(picosystem::hw.fb_copy)},
        devices::ScreenPalette{std::span(palette_copy)}
    );
}

std::span<const std::uint32_t, 32> get_default_palette()
{
    return ::video::pico8_palette_rgb8;
}

}