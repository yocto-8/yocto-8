#pragma once

#include <array>
#include <cstdint>
#include <cstdio>

namespace extmem
{

// TODO: this should make use of multiple cache pages, because we can afford it
//       this would make it possible to swap pages in the background (with some effort)
//       and avoid requiring huge transfers during cache misses

// START OF TWEAKABLES

constexpr std::size_t
    cache_size = 4096;

constexpr std::uintptr_t
    bank_base = 0x2F000000,
    bank_size = 8 * 1024 * 1024; // 8MB

constexpr bool
    access_debug = false;

// END OF TWEAKABLES

constexpr std::uintptr_t
    bank_last_byte = bank_base + bank_size - 1;

extern std::array<char, cache_size> cache;
extern std::uintptr_t active_cache_page_index;

inline void assert_address_within_bank(std::uintptr_t address)
{
    if constexpr (access_debug)
    {
        if (!((address >= bank_base) && (address <= bank_last_byte)))
        {
            printf("Could not recover from memory access at %#06x as it is not within the emulated bank\n", address);
            for (;;)
                ;
        }
    }
}

inline void swap_to_cache_page(std::uintptr_t cache_page_index)
{
    if (cache_page_index != active_cache_page_index)
    {
        //printf("faking swap to %d\n", cache_page_index);
        active_cache_page_index = cache_page_index;
    }
}

inline std::size_t load_cache_and_compute_address(std::uintptr_t address)
{
    const std::uintptr_t address_in_bank = address - bank_base;
    const std::size_t cache_page_index = address_in_bank / cache_size;
    const std::size_t address_in_page = address_in_bank % cache_size;

    swap_to_cache_page(cache_page_index);

    //printf("%d %d %d\n", address_in_bank, cache_page_index, address_in_page);

    return address_in_page;
}

inline std::uintptr_t resolve_address(std::uintptr_t address)
{
    return address - bank_base;
}

template<class T>
T& get_temporary_ref(std::uintptr_t address)
{
    assert_address_within_bank(address);
    return reinterpret_cast<T&>(cache[load_cache_and_compute_address(address)]);
}
}