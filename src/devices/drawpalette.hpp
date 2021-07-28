#pragma once

#include <cstdint>
#include <gsl/gsl>
#include <emu/mmio.hpp>

namespace devices
{

struct DrawPalette : emu::MMIODevice<16>
{
    using MMIODevice::MMIODevice;

    static constexpr std::uint16_t default_map_address = 0x5F00;

    void reset()
    {
        for (std::size_t i = 0; i < data.size(); ++i)
        {
            set_color(i, i);
        }

        set_transparent(0);
    }

    void set_color(std::uint8_t palette_index, std::uint8_t resolved_index)
    {
        data[palette_index] = resolved_index;
    }

    std::uint8_t get_color(std::uint8_t palette_index)
    {
        return data[palette_index];
    }

    void set_transparent(std::uint8_t palette_index, bool transparent = true)
    {
        data[palette_index] &= 0x0F;
        data[palette_index] |= int(transparent) << 4;
    }

    bool is_transparent(std::uint8_t palette_index)
    {
        return (data[palette_index] >> 4) != 0;
    }
};

}