#pragma once

#include "pico/platform.h"

#include <cstdint>

namespace arch::pico::extmem
{
extern "C"
{
[[gnu::naked, gnu::used]] // owo
void isr_hardfault();

[[gnu::used]]
void hard_fault_handler_c(std::uint32_t* args);
}
}