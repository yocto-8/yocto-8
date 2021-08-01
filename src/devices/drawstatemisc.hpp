#pragma once

#include <cstdint>
#include <gsl/gsl>
#include <emu/mmio.hpp>

namespace devices
{

struct DrawStateMisc : emu::MMIODevice<64>
{
    using MMIODevice::MMIODevice;

    static constexpr std::uint16_t default_map_address = 0x5F00;

    std::uint8_t raw_pen_color() const
    {
        return data[0x25];
    }

    std::uint16_t fill_pattern() const
    {
        return data[0x31] | (data[0x32] << 8);
    }

    bool fill_pattern_at(std::uint8_t x, std::uint8_t y) const
    {
        const auto bit_index = (x & 0b11) | ((y & 0b11) << 2);
        return (fill_pattern() >> bit_index) & 0b1;
    }

    bool fill_zero_is_transparent() const
    {
        return (data[0x33] & 0b1) == 0;
    }
};

}