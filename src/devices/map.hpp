#pragma once

#include <emu/mmio.hpp>

namespace devices
{

struct Map : emu::MMIODevice<8192>
{
    using MMIODevice::MMIODevice;

    // NOTE: the first 4096 bytes overlap with the bottom of the sprite sheet
    static constexpr std::uint16_t default_map_address = 0x1000;

    static constexpr std::size_t width = 128, height = 128;

    std::uint8_t& tile(std::uint8_t x, std::uint8_t y) const
    {
        return data[std::uintptr_t(x) + (std::uintptr_t(y) * width)];
    }
};

}