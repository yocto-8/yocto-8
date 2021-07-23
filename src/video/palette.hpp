#pragma once

#include <array>
#include <cstdint>

namespace video
{

constexpr std::array<std::uint32_t, 32> pico8_palette_rgb8 {
    0x000000, 0x1D2B53, 0x7E2553, 0x008751,
    0xAB5236, 0x5F574F/*0x6F675F*/, 0xC2C3C7, 0xFFF1E8,
    0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436,
    0x29ADFF, 0x83769C, 0xFF77A8, 0xFFCCAA,
    // TODO: add hidden palette colors
};

}