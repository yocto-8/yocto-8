#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <pico/platform.h>

namespace arch::pico::extmem::spiram
{

static constexpr int pin_rx = 8, pin_cs = 9, pin_sck = 10, pin_tx = 11;
static constexpr std::size_t ram_size = 8 * 1024 * 1024, page_size = 1024;

void setup();

//! \brief Send a receive a dummy page at address 0, to check whether the chip is present.
//! \warning This will destroy contents of the first page on the chip.
//! \returns true if the chip appears to be functional, false otherwise.
bool test_chip_presence_destructive();

void select(bool chip_selected);

void validate_address(std::uint32_t page_aligned_address);

void __not_in_flash_func(read_page)(std::uint32_t page_address, std::span<std::uint8_t, page_size> buf);
void __not_in_flash_func(write_page)(std::uint32_t page_address, std::span<const std::uint8_t, page_size> buf);

}