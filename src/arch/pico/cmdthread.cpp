#include "cmdthread.hpp"

#include <pico/multicore.h>
#include <pico/platform.h>

#include <emu/emulator.hpp>
#include <hardwarestate.hpp>
#include <devices/image.hpp>
#include <devices/screenpalette.hpp>

namespace arch::pico
{

// we make this live statically rather than on the core1 stack because of stack size concerns
static devices::Framebuffer::ClonedArray fb_copy;

[[gnu::flatten]]
void __scratch_x("core1_entry") core1_entry()
{
    for (;;)
    {
        const auto command = IoThreadCommand(multicore_fifo_pop_blocking());

        switch (command)
        {
        case IoThreadCommand::PUSH_FRAME:
        {
            emu::device<devices::Framebuffer>.clone_into(fb_copy);

            devices::ScreenPalette::ClonedArray palette_copy;
            emu::device<devices::ScreenPalette>.clone_into(palette_copy);

            multicore_fifo_push_blocking(0); // copy done - we can update in background now

            hw.ssd1351.update_frame(
                devices::Framebuffer{std::span(fb_copy)},
                devices::ScreenPalette{std::span(palette_copy)}
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