#include "cmdthread.hpp"
#include "pico/multicore.h"

#include <emu/emulator.hpp>
#include <hardwarestate.hpp>
#include <devices/image.hpp>
#include <devices/screenpalette.hpp>

namespace arch::pico
{

void core1_entry()
{
    for (;;)
    {
        const auto command = IoThreadCommand(multicore_fifo_pop_blocking());

        switch (command)
        {
        case IoThreadCommand::PUSH_FRAME:
        {
            auto fb_copy = emu::device<devices::Framebuffer>.clone();
            auto palette_copy = emu::device<devices::ScreenPalette>.clone();

            multicore_fifo_push_blocking(0); // copy done - we can update in background now

            hw.ssd1351.update_frame(
                devices::Framebuffer{gsl::span(fb_copy)},
                devices::ScreenPalette{gsl::span(palette_copy)}
            );
            
            break;
        }
        }
    }
}

std::uint32_t run_blocking_command(IoThreadCommand cmd)
{
    multicore_fifo_push_blocking(std::uint32_t(cmd));
    return multicore_fifo_pop_blocking();
}

}