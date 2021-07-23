#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <gsl/gsl>

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
    
    using View = gsl::span<std::uint8_t, frame_bytes>;

    void set_pixel(std::uint8_t x, std::uint8_t y, std::uint8_t palette_entry)
    {
        std::size_t i = y + (x * Framebuffer::frame_width);
        
        std::uint8_t& pixel_pair_byte = data[i / 2];

        if (i % 2 == 0) // lower pixel?
        {
            pixel_pair_byte &= 0xF0; // clear lower byte
            pixel_pair_byte |= palette_entry; // set lower byte
        }
        else
        {
            pixel_pair_byte &= 0x0F; // clear upper byte
            pixel_pair_byte |= palette_entry << 4; // set upper byte
        }
    }

    std::array<std::uint8_t, frame_bytes> data;
};

}