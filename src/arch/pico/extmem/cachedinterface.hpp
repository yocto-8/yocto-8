#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <extmem/spiram.hpp>

namespace arch::pico::extmem
{

constexpr std::size_t
    cache_size = spiram::page_size;

constexpr std::uintptr_t
    bank_base = 0x2F000000,
    bank_size = spiram::ram_size;

constexpr bool
    access_debug = true;

constexpr std::size_t
    cache_page_count = 16;

constexpr std::uintptr_t
    bank_last_byte = bank_base + bank_size - 1;

constexpr std::uint32_t
    invalid_bank = std::uint32_t(-1);

struct Cache
{
    using CachePage = std::array<std::uint8_t, cache_size>;

    alignas(4) std::array<CachePage, cache_page_count> pages;
    std::array<std::uint32_t, cache_page_count> active_page_indices;

#ifdef YOCTO8_EXTMEM_CHECKSUM
    using Checksum = std::uint8_t;
    static constexpr Checksum default_checksum = 69;
    std::array<Checksum, bank_size / cache_size> checksums;

    static constexpr Checksum compute_checksum(std::span<std::uint8_t, cache_size> cache_page)
    {
        std::uint32_t ret = 0;

        for (const auto b : cache_page)
        {
            ret = (ret + 1) ^ b;
        }

        return Checksum(ret);
    }
#endif

    constexpr Cache()
    {
        active_page_indices.fill(invalid_bank);
#ifdef YOCTO8_EXTMEM_CHECKSUM
        checksums.fill(default_checksum);
#endif
    }
};

extern Cache cache;

struct PageInfo
{
    PageInfo(std::uint32_t page_index) :
        index(page_index),
        cache_slot(page_index % cache.active_page_indices.size()),
        base_address(page_index * spiram::page_size)
    {}

    std::uint32_t index;
    std::uint32_t cache_slot;
    std::uint32_t base_address;
};

[[gnu::noinline]]
inline void swap_out(PageInfo page)
{
    spiram::write_page(page.base_address, cache.pages[page.cache_slot]);

#ifdef YOCTO8_EXTMEM_CHECKSUM
    cache.checksums[page.index] = Cache::compute_checksum(cache.pages[page.cache_slot]);
    //printf("chk %d\n", Cache::compute_checksum(cache.pages[page.cache_slot]));
#endif
}

[[gnu::noinline]]
inline void swap_in(PageInfo page)
{
    spiram::read_page(page.base_address, cache.pages[page.cache_slot]);
    cache.active_page_indices[page.cache_slot] = page.index;

#ifdef YOCTO8_EXTMEM_CHECKSUM
    const auto calculated_checksum = Cache::compute_checksum(cache.pages[page.cache_slot]);
    const auto expected_checksum = cache.checksums[page.index];

    if (expected_checksum != Cache::default_checksum && calculated_checksum != expected_checksum) [[unlikely]]
    {
        printf(
            "Page %#06x: expected checksum %#04x, just calculated %#04x\n",
            page.base_address,
            expected_checksum,
            calculated_checksum
        );

        for (;;)
            ;
    }

    printf("%d ok\n", page.index);

#endif
}

[[gnu::always_inline]]
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