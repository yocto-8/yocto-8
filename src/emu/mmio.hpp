#pragma once

#include <array>
#include <gsl/gsl>

namespace emu
{

template<std::size_t MapLength>
struct MMIODevice
{
    static constexpr auto map_length = MapLength;
    using View = gsl::span<std::uint8_t, map_length>;

    explicit constexpr MMIODevice(View data) :
        data(data)
    {}

    constexpr std::array<std::uint8_t, map_length> clone() const
    {
        std::array<std::uint8_t, map_length> ret{};
        std::memcpy(ret.data(), data.data(), data.size());
        return ret;
    }

    View data;
};

struct Memory : MMIODevice<65536>
{
    using MMIODevice::MMIODevice;
    static constexpr std::uint16_t map_address = 0;
};

}