#include <platform/platform.hpp>
#include <platform/asupico/asupico.hpp>

#include <pico/stdlib.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>

#include <extmem/cachedinterface.hpp>
#include <hardwarestate.hpp>

namespace arch::pico::platform
{

void init_hardware()
{
    using namespace asupico;

    init_default_frequency();
    init_stdio();
    init_buttons();
    init_spi_ram();
    init_emulator();
    init_cmd_thread();
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

}