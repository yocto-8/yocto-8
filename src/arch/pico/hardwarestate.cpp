#include "hardwarestate.hpp"

#include <hardware/clocks.h>
#include <hardware/vreg.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>

#include <cmdthread.hpp>
#include <emu/emulator.hpp>

namespace arch::pico {

void init_cmd_thread() { multicore_launch_core1(core1_entry); }

ArchState state;

} // namespace arch::pico