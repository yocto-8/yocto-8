#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <gsl/gsl>

namespace video
{

class FramebufferView
{
    public:
    static constexpr std::size_t
        frame_width = 128,
        frame_height = 128,
        frame_pixels_per_byte = 2,
        frame_bytes = (frame_width * frame_height) / frame_pixels_per_byte;

    using View = gsl::span<std::uint8_t, frame_bytes>;

    constexpr FramebufferView(View data) :
        data(data)
    {}

    void clear(std::uint8_t palette_entry)
    {
        std::uint8_t pixel_pair_byte = palette_entry | (palette_entry << 4);
        std::fill(data.begin(), data.end(), pixel_pair_byte);
    }

    void set_pixel(std::uint8_t x, std::uint8_t y, std::uint8_t palette_entry)
    {
        std::size_t i = x + (y * FramebufferView::frame_width);
        
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

    std::uint8_t get_pixel(std::uint8_t x, std::uint8_t y)
    {
        std::size_t i = x + (y * FramebufferView::frame_width);

        std::uint8_t pixel_pair_byte = data[i / 2];

        if (i % 2 == 0) // lower pixel?
        {
            return pixel_pair_byte & 0x0F;
        }

        return pixel_pair_byte >> 4;
    }

    View data;
};

}