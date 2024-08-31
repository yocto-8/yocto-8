#pragma once

#include <cstdint>

#include <pico/multicore.h>

namespace arch::pico {

enum class IoThreadCommand : std::uint32_t { PUSH_FRAME };

void core1_entry();

std::uint32_t run_blocking_command(IoThreadCommand cmd);

} // namespace arch::pico