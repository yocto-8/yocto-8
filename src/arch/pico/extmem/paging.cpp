#include "paging.hpp"
#include "pico/platform.h"
#include "pico/time.h"
#include <cstdio>
#include <cstring>
#include <extmem/spiram.hpp>

#include <hardware/structs/xip_ctrl.h>
#include <hardware/timer.h>
#include <hardware/structs/systick.h>
#include <RP2040.h>

//#define FILL_MY_STDIO_WITH_DEBUG_UWU

namespace arch::pico::extmem
{

std::array<Page, page_cache_size> page_cache;

std::array<PageIndex, page_cache_size> page_cache_occupants;
std::array<PageIndex, used_mpu_regions.size()> mpu_region_occupants;

PerfCounters perf_counters;

static bool xip_enabled;

void __not_in_flash_func(init_xipram)()
{
    page_cache_occupants.fill(nonpresent_page);
    mpu_region_occupants.fill(nonpresent_page);
    xip_enabled = true;
    perf_counters = {};

    // enable the MPU, keep it enabled during hardfaults, and use the background memory map
    // https://www.keil.com/pack/doc/CMSIS_Dev/Core/html/group__mpu__functions.html#ga31406efd492ec9a091a70ffa2d8a42fb
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    // when the XIP is active, it already kinda does that, because it would trigger a bus error when
    // accessing the XIP RAM bank while the XIP cache is active
    // but whatever -- we need this region set in RAM mode, anyway
    mpu_enable_fault_on_xipram();

    // configure hardware clock
    hardware_alarm_claim(xip_reenable_clock_num);
    hardware_alarm_set_callback(xip_reenable_clock_num, +[]([[maybe_unused]] unsigned alarm_id) {
        //ARM_MPU_SetRegionEx(7UL, XIP_BASE, ARM_MPU_RASR(0UL, ARM_MPU_AP_NONE, 0UL, 0UL, 1UL, 1UL, 0x00UL, ARM_MPU_REGION_SIZE_64MB));
        enable_xip_mode();
    });
}

bool __not_in_flash_func(is_in_ram_mode)()
{
    return !xip_enabled;
}

[[gnu::noinline]]
void __not_in_flash_func(enable_xip_mode)()
{
#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("=== XIP MODE\n");
#endif

    hardware_alarm_cancel(xip_reenable_clock_num);
    
    flush_xipram();

    // enable XIP
    xip_ctrl_hw->ctrl = 0b0101;
    xip_ctrl_hw->flush = 1;
    xip_ctrl_hw->flush;
    
    xip_enabled = true;

    ++perf_counters.xip_swaps;

#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("=== XIP OK\n");
#endif
}

[[gnu::noinline]]
void __not_in_flash_func(enable_ram_mode)()
{
#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("=== RAM MODE\n");
#endif

    // disable XIP
    xip_ctrl_hw->ctrl = 0b0100;

    // if a flush is in progress, wait for it to end.
    // otherwise, XIP RAM contents could get corrupted as it updates cacheline contents.
    xip_ctrl_hw->flush;

    xip_enabled = false;
}

void __not_in_flash_func(flush_xipram)()
{
#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("--- flush_xipram()\n");
#endif

    // NOTE: bear in mind that pages in RAM could be bound to the same cache slot

    for (MpuRegionIndex i = 0; i < used_mpu_regions.size(); ++i)
    {
        if (mpu_region_occupants[i] != nonpresent_page)
        {
            evict_from_xipram(i);
            unclaim_mpu_region(i);
        }
    }
}

void __not_in_flash_func(evict_from_cache_slot)(PageCacheSlot slot)
{
    const auto page_index = page_cache_occupants[slot];
    const auto cache_slot = cache_slot_from_page_index(page_index);
    const auto extmem_address = extmem_address_from_page_index(page_index);

#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("--> evicting slot %d to extmem %p\n", slot, extmem_address);
#endif
    
    spiram::write_page(
        extmem_address,
        std::span<const std::uint8_t, page_size>(
            reinterpret_cast<const std::uint8_t*>(page_cache[cache_slot].data()),
            page_size));

    page_cache_occupants[slot] = nonpresent_page;

    ++perf_counters.page_cache_evicts;
}

PageCacheSlot __not_in_flash_func(evict_from_xipram)(MpuRegionIndex region)
{
    const auto page_index = mpu_region_occupants[region];
    const auto base_address = address_from_page_index(page_index);
    const auto cache_slot = cache_slot_from_page_index(page_index);

    if (page_cache_occupants[cache_slot] != nonpresent_page)
    {
        evict_from_cache_slot(cache_slot);
    }

    ++perf_counters.region_evicts;

#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("--> evicting region %d (holds page %d) from %p to slot %d\n", region, page_index, base_address, cache_slot);
#endif

    std::memcpy(page_cache[cache_slot].data(), base_address, page_size);
    page_cache_occupants[cache_slot] = page_index;
    mpu_region_occupants[region] = nonpresent_page;

    return cache_slot;
}

void __not_in_flash_func(load_page_from_extmem)(PageIndex page, std::byte* target)
{
    const auto extmem_address = extmem_address_from_page_index(page);

#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("<-- loading from extmem %p to %p\n", page, target);
#endif

    spiram::read_page(
        extmem_address,
        std::span<std::uint8_t, page_size>(reinterpret_cast<std::uint8_t*>(target), page_size));
}

bool __not_in_flash_func(try_load_page_from_cache)(PageIndex page, std::byte* target)
{
    const auto cache_slot = cache_slot_from_page_index(page);

    if (page != page_cache_occupants[cache_slot])
    {
        ++perf_counters.page_cache_misses;
        return false;
    }
    
#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("<-- loading from pagecache %p to %p\n", page, target);
#endif

    std::memcpy(target, page_cache[cache_slot].data(), page_size);
    ++perf_counters.page_cache_hits;
    return true;
}

void __not_in_flash_func(reclaim_mpu_region_for)(PageIndex page_index)
{
    const auto region_index = region_index_from_page_index(page_index);
    const auto base_address = address_from_page_index(page_index);
    const auto previous_occupant = mpu_region_occupants[region_index];

#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    printf("--- reclaim region %d for page %d\n", region_index, page);
#endif

    if (previous_occupant != nonpresent_page)
    {
        evict_from_xipram(region_index);
    }

    // the region at index 0 traps all accesses to the bank area
    // set up any region with a higher priority (= higher index)
    // that allows them for a 1KiB window, which is the "active page"
    ARM_MPU_SetRegionEx(
        used_mpu_regions[region_index],
        std::uintptr_t(base_address),
        ARM_MPU_RASR(0UL, ARM_MPU_AP_FULL, 0UL, 0UL, 1UL, 1UL, 0x00UL, mpu_page_size));

    mpu_region_occupants[region_index] = page_index;
}

void __not_in_flash_func(unclaim_mpu_region)(MpuRegionIndex region_index)
{
    ARM_MPU_ClrRegion(used_mpu_regions[region_index]);
}

bool __not_in_flash_func(is_paged)(std::uintptr_t address)
{
    const auto page_index = page_index_from_address(reinterpret_cast<std::byte*>(address));
    const auto region_index = region_index_from_page_index(page_index);
    return mpu_region_occupants[region_index] == page_index;
}

void __not_in_flash_func(page_in)(std::uintptr_t address)
{
    systick_hw->csr = 0b101; // enable systick timer, no exception request, processor clock
    systick_hw->rvr = 0x00FFFFFF; // reset value when counter reaches 0
    systick_hw->cvr = 0;

    const auto page_index = page_index_from_address(reinterpret_cast<std::byte*>(address));
    const auto base_address = address_from_page_index(page_index);

#ifdef FILL_MY_STDIO_WITH_DEBUG_UWU
    if (page_index >= bank_size / page_size)
    {
        printf("!!! tried to page_in() out of bounds page: %d, fault at address %p\n", page_index, base_address);
        __breakpoint();
    }

    printf("~~~ page in %p, base address %p\n", page_index, base_address);
#endif
    reclaim_mpu_region_for(page_index);

    if (!try_load_page_from_cache(page_index, base_address))
    {
        load_page_from_extmem(page_index, base_address);
    }

    // NOTE: the µs value is pretty janky, and was obtained through trial and error for the most part
    //       it seems to trigger quite sooner than the said xµs, too
    // TODO: raw logic; bc this is unnecessarily slow for this usecase
    hardware_alarm_set_target(xip_reenable_clock_num, delayed_by_us(get_absolute_time(), 500));
    
    perf_counters.ticks_in_page_load += 0x00FFFFFF - systick_hw->cvr;
    ++perf_counters.page_loads;

    if (perf_counters.page_cache_evicts % 400 == 0)
    {
        printf("perf: %d/%d pagecache hits, %d pagecache evicts, %d region evicts, %d XIP swaps, avg %d ticks/page load\n",
            perf_counters.page_cache_hits,
            perf_counters.page_cache_misses + perf_counters.page_cache_hits,
            perf_counters.page_cache_evicts,
            perf_counters.region_evicts,
            perf_counters.xip_swaps,
            int(perf_counters.ticks_in_page_load / perf_counters.page_loads));
    }
}

void __not_in_flash_func(mpu_enable_fault_on_xipram)()
{
    ARM_MPU_SetRegionEx(
        xipram_protection_mpu_region,
        std::uintptr_t(bank_base),
        ARM_MPU_RASR(0UL, ARM_MPU_AP_NONE, 0UL, 1UL, 1UL, 1UL, 0x00UL, ARM_MPU_REGION_SIZE_16MB));
}

MpuRegionIndex region_index_from_page_index(PageIndex page_index)
{
    const auto index_within_16kb_region = page_index % (16384 / page_size);
    return index_within_16kb_region % used_mpu_regions.size();
}

PageCacheSlot __not_in_flash_func(cache_slot_from_page_index)(PageIndex page_index)
{
    return page_index % page_cache.size();
}

PageIndex __not_in_flash_func(page_index_from_address)(std::byte* xipram_address)
{
    return (std::uintptr_t(xipram_address) - std::uintptr_t(bank_base)) / page_size;
}

std::byte* __not_in_flash_func(address_from_page_index)(PageIndex page_index)
{
    return reinterpret_cast<std::byte*>((page_index * page_size) + bank_base);
}

std::uint32_t __not_in_flash_func(extmem_address_from_page_index)(PageIndex page_index)
{
    return page_index * page_size;
}

}