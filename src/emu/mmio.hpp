#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <span>

namespace emu
{

template<std::size_t MapLength>
struct MMIODevice
{
    static constexpr auto map_length = MapLength;
    using View = std::span<std::uint8_t, map_length>;

    explicit constexpr MMIODevice(View data) :
        data(data)
    {}

    constexpr std::array<std::uint8_t, map_length> clone() const
    {
        std::array<std::uint8_t, map_length> ret;
        std::memcpy(ret.data(), data.data(), data.size());
        return ret;
    }

    View data;
};

struct Memory : MMIODevice<65536>
{
    using MMIODevice::MMIODevice;
    static constexpr std::uint16_t map_address = 0;

    template<class T>
    T peek_le(std::uint16_t addr)
    {
        T ret = 0;
        for (std::size_t i = 0; i < sizeof(addr); ++i)
        {
            ret |= data[addr + i] << (i * 8);
        }
        return ret;
    }

    template<class T>
    void poke_le(std::uint16_t addr, T value)
    {
        for (std::size_t i = 0; i < sizeof(addr); ++i)
        {
            data[addr + i] = (value >> (i * 8)) & 0xFF;
        }
    }

    void memcpy(std::uint16_t dst, std::uint16_t src, std::size_t bytes)
    {
        std::memmove(&data[dst], &data[src], bytes);
    }

    void memset(std::uint16_t dst, std::uint16_t val, std::size_t bytes)
    {
        std::memset(&data[dst], val, bytes);
    }
};

}