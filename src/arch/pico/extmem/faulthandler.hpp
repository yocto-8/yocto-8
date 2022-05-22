#pragma once

#include "hardware/regs/addressmap.h"
#include "pico/platform.h"

#include <cstddef>
#include <cstdint>

namespace arch::pico::extmem
{
extern "C"
{
[[gnu::naked, gnu::used]] // owo
void __not_in_flash_func(isr_hardfault());

[[gnu::used]]
void __not_in_flash_func(hard_fault_handler_c)(std::uint32_t* args);
}
}