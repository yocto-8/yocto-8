#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <extmem/spiram.hpp>

namespace extmem
{

constexpr std::size_t
    cache_size = spiram::page_size;

constexpr std::uintptr_t
    bank_base = 0x2F000000,
    bank_size = spiram::ram_size;

constexpr bool
    access_debug = true;

constexpr std::size_t
    cache_page_count = 32;

constexpr std::uintptr_t
    bank_last_byte = bank_base + bank_size - 1;

constexpr std::uint16_t
    invalid_bank = std::uint16_t(-1);

struct Cache
{
    using CachePage = std::array<std::uint8_t, cache_size>;

    std::array<CachePage, cache_page_count> pages;
    std::array<std::uint16_t, cache_page_count> active_page_indices;

    Cache()
    {
        active_page_indices.fill(invalid_bank);
    }
};

extern Cache cache;

struct PageInfo
{
    PageInfo(std::uint16_t page_index) :
        index(page_index),
        cache_slot(page_index % cache.active_page_indices.size()),
        base_address(page_index * spiram::page_size)
    {}

    std::uint16_t index;
    std::uint16_t cache_slot;
    std::uintptr_t base_address;
};

inline void swap_out(PageInfo page)
{
    spiram::write_page(page.base_address, cache.pages[page.cache_slot]);
}

inline void swap_in(PageInfo page)
{
    spiram::read_page(page.base_address, cache.pages[page.cache_slot]);
    cache.active_page_indices[page.cache_slot] = page.index;
}

inline char* get_raw_temporary_ref(std::uintptr_t address)
{
    const std::uintptr_t address_in_bank = address - bank_base;
    const std::size_t address_in_page = address_in_bank % cache_size;

    const PageInfo page(address_in_bank / cache_size);

    const auto previous_page_in_slot = cache.active_page_indices[page.cache_slot];

    if (page.index != previous_page_in_slot) [[unlikely]]
    {
        // perform the address bounds check inside of this cold path is faster
        // this should be somewhat safe and cozy to do in here, as the slot should never match if the address is bad
        //
        // i'm actually not quite sure how true that is because index is a uint16_t,
        // but this probably won't cause too much trouble anyway
        if constexpr (access_debug)
        {
            if (!((address >= bank_base) && (address <= bank_last_byte))) [[unlikely]]
            {
                printf("Could not recover from memory access at %#06x as it is not within the emulated bank\n", address);
                for (;;)
                    ;
            }
        }

        if (previous_page_in_slot != invalid_bank)
        {
            swap_out(cache.active_page_indices[page.cache_slot]);
        }

        swap_in(page.index);
    }

    return reinterpret_cast<char*>(&cache.pages[page.cache_slot][address_in_page]);
}

inline std::uintptr_t resolve_address(std::uintptr_t address)
{
    return address - bank_base;
}

template<class T>
T& get_temporary_ref(std::uintptr_t address)
{
    return *reinterpret_cast<T*>(get_raw_temporary_ref(address));
}
}