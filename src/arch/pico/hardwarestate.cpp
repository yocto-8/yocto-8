#include "hardwarestate.hpp"

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>

#include <emu/emulator.hpp>
#include <extmem/cachedinterface.hpp>
#include <cmdthread.hpp>

namespace arch::pico
{

void init_cmd_thread()
{
    multicore_launch_core1(core1_entry);
}

ArchState state;

}