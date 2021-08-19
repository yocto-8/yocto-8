#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace video
{

constexpr std::array<std::uint32_t, 32> pico8_palette_rgb8 {
    0x000000, 0x1D2B53, 0x7E2553, 0x008751,
    0xAB5236, 0x5F574F, 0xC2C3C7, 0xFFF1E8,
    0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436,
    0x29ADFF, 0x83769C, 0xFF77A8, 0xFFCCAA,
    // TODO: add hidden palette colors
};

constexpr std::uint32_t pack_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

constexpr std::array<std::uint32_t, 32> pico8_precal_palette_rgb8 {
    pack_rgb(0, 0, 0),
    pack_rgb(29, 43, 71),
    pack_rgb(109, 5, 42),
    pack_rgb(0, 121, 54),
    pack_rgb(149, 70, 45),
    pack_rgb(68, 72, 69),
    pack_rgb(144, 144, 216),
    pack_rgb(222, 210, 216),
    pack_rgb(255, 25, 68),
    pack_rgb(255, 148, 0),
    pack_rgb(255, 250, 17),
    pack_rgb(0, 255, 81),
    pack_rgb(32, 144, 255),
    pack_rgb(99, 88, 165),
    pack_rgb(238, 25, 159),
    pack_rgb(240, 179, 133)
};

constexpr auto default_palette_rgb8 = pico8_precal_palette_rgb8;

}