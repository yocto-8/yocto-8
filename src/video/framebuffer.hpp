#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace video
{

class Framebuffer
{
    public:
    static constexpr std::size_t
        frame_width = 128,
        frame_height = 128,
        frame_pixels_per_byte = 2,
        frame_bytes = (frame_width * frame_height) / frame_pixels_per_byte;

    std::array<std::uint8_t, frame_bytes> data;
};

}