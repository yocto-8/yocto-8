#pragma once

#include "pico/platform.h"

#include <cstdint>

namespace extmem
{
extern "C"
{
[[gnu::naked, gnu::used]] // owo
void __not_in_flash_func(isr_hardfault)();

[[gnu::used, gnu::flatten]]
void __not_in_flash_func(hard_fault_handler_c)(std::uint32_t* args);
}
}