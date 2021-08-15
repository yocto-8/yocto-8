#pragma once

#include <cstdint>
#include <emu/mmio.hpp>

namespace devices
{

struct ClippingRectangle : emu::MMIODevice<4>
{
    using MMIODevice::MMIODevice;

    static constexpr std::uint16_t default_map_address = 0x5F20;

    void reset() const
    {
        x_begin() = 0;
        y_begin() = 0;
        x_end() = 128;
        y_end() = 128;
    }

    std::uint8_t& x_begin() const
    {
        return data[0];
    }

    std::uint8_t& y_begin() const
    {
        return data[1];
    }

    std::uint8_t& x_end() const
    {
        return data[2];
    }

    std::uint8_t& y_end() const
    {
        return data[3];
    }

    std::uint8_t width() const
    {
        if (x_end() > x_begin())
        {
            return 0;
        }

        return x_end() - x_begin();
    }

    std::uint8_t height() const
    {
        if (y_end() > y_begin())
        {
            return 0;
        }

        return y_end() - y_begin();
    }

    bool contains(int x, int y) const
    {
        return x >= int(x_begin()) && y >= int(y_begin())
            && x <= int(x_end()) && y <= int(y_end());
    }
};

}