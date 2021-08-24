#pragma once

#include <cstdint>
#include <emu/mmio.hpp>
#include <util/point.hpp>

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

    util::Point top_left() const
    {
        return {int(x_begin()), int(y_begin())};
    }

    std::uint8_t& x_end() const
    {
        return data[2];
    }

    std::uint8_t& y_end() const
    {
        return data[3];
    }

    util::Point bottom_right() const
    {
        return {int(x_end()), int(y_end())};
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

    bool contains(util::Point p) const
    {
        return p.x >= 0 && p.y >= 0
            && p.x <= 127 && p.y <= 127
            && p.x >= int(x_begin()) && p.y >= int(y_begin())
            && p.x <= int(x_end()) && p.y <= int(y_end());
    }
};

}