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

void mpu_setup();

bool is_paged(std::uintptr_t unaligned_address);
void mpu_to_xip_mode();
void mpu_to_ram_mode();

void page_in(std::uintptr_t address);
void page_out();

inline std::byte *const bank_base = reinterpret_cast<std::byte*>(XIP_SRAM_BASE);
inline const std::size_t bank_size = 8388608;

}