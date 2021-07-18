#pragma once

#include <cstddef>
#include <gsl/gsl>

namespace extmem::spiram
{

static constexpr int pin_rx = 8, pin_cs = 9, pin_sck = 10, pin_tx = 11;
static constexpr std::size_t ram_size = 8 * 1024 * 1024, page_size = 1024, burst_size = 32;

void setup();

void select(bool chip_selected);

void validate_address(std::uint32_t page_aligned_address);

void read_page(std::uint32_t page_address, gsl::span<std::uint8_t, page_size> buf);
void write_page(std::uint32_t page_address, gsl::span<const std::uint8_t, page_size> buf);

}