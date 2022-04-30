#pragma once

#include <cstdint>
#include <concepts>
#include <span>

namespace util
{

constexpr std::uint32_t r8g8b8_to_r5g6b5(std::uint32_t r8g8b8)
{
    std::uint8_t
        r5 = ((r8g8b8 >> 16) & 0xFF) >> (8 - 5),
        g6 = ((r8g8b8 >>  8) & 0xFF) >> (8 - 6),
        b5 = ((r8g8b8 >>  0) & 0xFF) >> (8 - 5);
            
    return (r5 << (5 + 6)) | (g6 << 5) | b5;
}

constexpr std::array<std::uint16_t, 32> make_r5g6b5_palette(std::span<const std::uint32_t, 32> palette, bool swap_bytes)
{
    std::array<std::uint16_t, 32> ret{};

    for (std::size_t i = 0; i < 32; ++i)
    {
        const std::uint32_t r8g8b8 = palette[i];
        const std::uint16_t r5g6b5 = util::r8g8b8_to_r5g6b5(r8g8b8);

        if (swap_bytes)
        {
            // Swap LSB/MSB
            ret[i] = ((r5g6b5 & 0xFF) << 8) | (r5g6b5 >> 8);
        }
        else {
            ret[i] = r5g6b5;
        }
    }

    return ret;
}
}