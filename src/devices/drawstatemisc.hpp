#pragma once

#include <cstdint>
#include <emu/mmio.hpp>

namespace devices
{

// TODO: the amount of manual bit logic is terrifying, pls fix

struct DrawStateMisc : emu::MMIODevice<64>
{
    using MMIODevice::MMIODevice;

    static constexpr std::uint16_t default_map_address = 0x5F00;

    std::uint8_t raw_pen_color() const
    {
        return data[0x25];
    }

    std::uint16_t fill_pattern() const
    {
        return data[0x31] | (data[0x32] << 8);
    }

    bool fill_pattern_at(std::uint8_t x, std::uint8_t y) const
    {
        const auto bit_index = (x & 0b11) | ((y & 0b11) << 2);
        return (fill_pattern() >> bit_index) & 0b1;
    }

    bool fill_zero_is_transparent() const
    {
        return (data[0x33] & 0b1) != 0;
    }

    std::int16_t get_camera_x() const
    {
        return data[0x28] | (data[0x29] << 8);
    }

    void set_camera_x(std::int16_t x) const
    {
        data[0x28] = x & 0xFF;
        data[0x29] = x >> 8;
    }

    std::int16_t get_camera_y() const
    {
        return data[0x2A] | (data[0x2B] << 8);
    }

    void set_camera_y(std::int16_t x) const
    {
        data[0x2A] = x & 0xFF;
        data[0x2B] = x >> 8;
    }

    void set_camera_position(std::int16_t x, std::int16_t y) const
    {
        set_camera_x(x);
        set_camera_y(y);
    }

    bool is_line_endpoint_valid() const
    {
        return data[0x35] == 0;
    }

    void set_line_endpoint_valid(bool valid) const
    {
        data[0x35] = !valid;
    }

    std::int16_t line_endpoint_x() const
    {
        return data[0x3c] | (data[0x3d] << 8);
    }

    void set_line_endpoint_x(std::int16_t x) const
    {
        data[0x3c] = x & 0xFF;
        data[0x3d] = x >> 8;
    }

    std::int16_t line_endpoint_y() const
    {
        return data[0x3e] | (data[0x3f] << 8);
    }

    void set_line_endpoint_y(std::int16_t y) const
    {
        data[0x3e] = y & 0xFF;
        data[0x3f] = y >> 8;
    }

    void set_line_endpoint(std::int16_t x, std::int16_t y) const
    {
        set_line_endpoint_valid(true);
        set_line_endpoint_x(x);
        set_line_endpoint_y(y);
    }
};

}