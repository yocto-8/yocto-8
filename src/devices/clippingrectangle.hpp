#pragma once

#include <cstdint>
#include <gsl/gsl>
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

    bool contains(std::uint8_t x, std::uint8_t y) const
    {
        return x >= x_begin() && x < x_end()
            && y >= y_begin() && y < y_end();
    }
};

}