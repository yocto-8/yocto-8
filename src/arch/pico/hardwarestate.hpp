#pragma once

#include "pico/types.h"
#include <array>

#include <io/pushbutton.hpp>
#include <pico/time.h>

namespace arch::pico {

/// \brief Hardware state shared across all pico-compatible platforms.
struct ArchState {
	absolute_time_t timer_start;
};

extern ArchState state;

// also put here initialization functions that are shared across all pico-based
// devices

void init_cmd_thread();

} // namespace arch::pico