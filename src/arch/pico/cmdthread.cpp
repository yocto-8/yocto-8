#include "cmdthread.hpp"

#include <pico/multicore.h>
#include <pico/platform.h>

#include <emu/emulator.hpp>
#include <hardwarestate.hpp>
#include <devices/image.hpp>
#include <devices/screenpalette.hpp>
#include <platform/platform.hpp>

namespace arch::pico
{

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
            platform::present_frame([] {
                multicore_fifo_push_blocking(0); // copy done - we can update in background now
            });
            
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