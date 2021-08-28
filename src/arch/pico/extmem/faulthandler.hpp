#pragma once

#include "pico/platform.h"

#include <cstdint>

namespace arch::pico::extmem
{
extern "C"
{
[[gnu::naked, gnu::used]] // owo
void __scratch_y("isr_hardfault") isr_hardfault();

[[gnu::used]]
void __scratch_y("hard_fault_handler_c") hard_fault_handler_c(std::uint32_t* args);
}
}