#pragma once

#include <cstdint>
#include <concepts>
#include <span>

namespace util
{

template<class T>
    requires std::regular<T>
constexpr T from_le(std::span<const std::uint8_t, sizeof(T)> memory)
{
    T ret{};
    for (std::size_t i = 0; i < memory.size(); ++i)
    {
        ret |= memory[i] << (i * 8);
    }
    return ret;
}

template<class T>
    requires std::regular<T>
constexpr void to_le(T value, std::span<std::uint8_t, sizeof(T)> memory)
{
    for (std::size_t i = 0; i < memory.size(); ++i)
    {
        memory[i] = std::uint8_t(value >> (i * 8));
    }
}

template<class T>
    requires std::regular<T>
class UnalignedLEWrapper
{
    public:
    using ByteView = std::span<std::uint8_t, sizeof(T)>;

    constexpr UnalignedLEWrapper(ByteView view) :
        m_view(view)
    {}

    UnalignedLEWrapper& operator=(T value)
    {
        to_le(value, m_view);
        return *this;
    }

    constexpr operator T() const
    {
        return from_le<T>(m_view);
    }

    private:
    const ByteView m_view;
};

}