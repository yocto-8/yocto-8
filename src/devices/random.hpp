#pragma once

#include <cstdint>
#include <emu/mmio.hpp>

namespace devices
{

struct Random : emu::MMIODevice<8>
{
    using MMIODevice::MMIODevice;

    static constexpr std::uint16_t default_map_address = 0x5F44;
};

}