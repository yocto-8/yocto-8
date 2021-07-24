#include "cmdthread.hpp"
#include "pico/multicore.h"

#include <emu/emulator.hpp>
#include <hardwarestate.hpp>

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
            const auto fb_view = emu::emulator.frame_buffer().data;

            std::array<std::uint8_t, 8192> fb_copy;
            // for whatever reason, memcpy is way faster than std::move/copy here
            //std::move(fb_view.begin(), fb_view.end(), fb_copy.begin());
            memcpy(fb_copy.data(), fb_view.data(), fb_view.size());

            multicore_fifo_push_blocking(0); // memcpy done - we can update in background now

            hw.ssd1351.update_frame(fb_copy);
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