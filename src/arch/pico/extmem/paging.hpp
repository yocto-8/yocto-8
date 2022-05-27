#pragma once

#include "extmem/spiram.hpp"
#include "hardware/regs/addressmap.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <RP2040.h>

namespace arch::pico::extmem
{

struct PerfCounters
{
    std::uint32_t page_cache_hits;
    std::uint32_t page_cache_misses;
    std::uint32_t page_cache_evicts;
    std::uint32_t region_evicts;
    std::uint32_t xip_swaps;

    std::uint64_t page_loads;
    std::uint64_t ticks_in_page_load;
};

constexpr auto page_cache_size = 36;
constexpr auto page_size = spiram::page_size;

static_assert(page_size == 1024); // NOTE: before changing this, make sure you update mpu_page_size
constexpr auto mpu_page_size = ARM_MPU_REGION_SIZE_1KB;

/// \brief Clock used to trigger a XIP enablement after some time
/// We do this to prevent excessive thrashing.
/// Swapping from XIP to RAM cache modes is very costly for now, more so than running XIP-free for a while.
constexpr unsigned xip_reenable_clock_num = 0;

constexpr std::uint8_t xipram_protection_mpu_region = 0;
constexpr std::array<std::uint8_t, 7> used_mpu_regions = {1, 2, 3, 4, 5, 6, 7};

using Page = std::array<std::byte, page_size>;
using PageIndex = unsigned;
using PageCacheSlot = unsigned;
using MpuRegionIndex = unsigned;

constexpr PageIndex nonpresent_page = PageIndex(-1);

inline std::byte *const bank_base = reinterpret_cast<std::byte*>(XIP_SRAM_BASE);
constexpr std::size_t bank_size = 8388608;

extern std::array<Page, page_cache_size> page_cache;

extern std::array<PageIndex, page_cache_size> page_cache_occupants;
extern std::array<PageIndex, used_mpu_regions.size()> mpu_region_occupants;

extern PerfCounters perf_counters;

/// \brief Initializes the MPU mechanism. Defaults to XIP flash mode.
void init_xipram();

/// \brief Whether XIP cache is disabled for XIP RAM mapping usage.
bool is_in_ram_mode();

/// \brief Enables XIP cache after flushing XIP RAM.
void enable_xip_mode();

/// \brief Enables RAM mode after flushing the entirety of Flash cache.
/// Flash accesses still work in RAM mode, but they will be slow as they will be uncached.
void enable_ram_mode();

/// \brief Evicts all the MPU-mapped pages in the XIP RAM segment into the \p page_cache.
void flush_xipram();

/// \brief Evicts a cached page into external memory. The slot is then marked as unused.
void evict_from_cache_slot(PageCacheSlot slot);

/// \brief Evicts a MPU-mapped page into the \p page_cache.
PageCacheSlot evict_from_xipram(MpuRegionIndex index);

/// \brief Loads a page from external memory and writes it to \p target.
void load_page_from_extmem(PageIndex page, std::byte* target);

/// \brief Tries to load a page from \p page_cache if it is cached.
/// \returns true if the page was successfully loaded and \p target was written to, false otherwise.
bool try_load_page_from_cache(PageIndex page, std::byte* target);

/// \brief Claims the MPU region region bound to a given page.
/// If said MPU region is claimed already, its contents will be evicted, updating metadata.
void reclaim_mpu_region_for(PageIndex page);

/// \brief Unclaims a mapped MPU region.
/// This does not evict the pointed memory, which is assumed to be handled by the caller.
void unclaim_mpu_region(MpuRegionIndex region_index);

bool is_paged(std::uintptr_t address);

/// \brief Maps the XIP RAM page associated with the address in memory,
/// loading its contents from cache or external memory.
/// \warning This assumes that the page is not currently mapped.
/// Failing to ensure this can result in coherency issues.
void page_in(std::uintptr_t address);

/// \brief Configures region \p xipram_protection_mpu_region to fault on accesses to the XIP RAM.
void mpu_enable_fault_on_xipram();

PageCacheSlot cache_slot_from_page_index(PageIndex page_index);
PageIndex page_index_from_address(std::byte* xipram_address);
std::byte* address_from_page_index(PageIndex page_index);
std::uint32_t extmem_address_from_page_index(PageIndex page_index);

}