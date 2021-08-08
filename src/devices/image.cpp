#include "image.hpp"

#include <algorithm>

namespace devices
{
    
void Image::clear(std::uint8_t palette_entry) const
{
    std::uint8_t pixel_pair_byte = palette_entry | (palette_entry << 4);
    std::ranges::fill(data, pixel_pair_byte);
}

void Image::set_nibble(std::size_t i, std::uint8_t palette_entry) const
{
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

void Image::set_pixel(std::uint8_t x, std::uint8_t y, std::uint8_t palette_entry) const
{
    set_nibble(x + (y * frame_width), palette_entry);
}

std::uint8_t Image::get_nibble(std::size_t i) const
{
    std::uint8_t pixel_pair_byte = data[i / 2];

    if (i % 2 == 0) // lower pixel?
    {
        return pixel_pair_byte & 0x0F;
    }

    return pixel_pair_byte >> 4;
}

std::uint8_t Image::get_pixel(std::uint8_t x, std::uint8_t y) const
{
    return get_nibble(x + (y * frame_width));
}

}