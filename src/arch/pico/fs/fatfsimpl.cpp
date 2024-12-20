#include <cstring>
#include <ff.h>

// diskio.h include must be after ff.h
#include <diskio.h>

#include "types.hpp"
#include <arch/pico/cmdthread.hpp>

#include <cstdio>
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <hardware/xip_cache.h>
#include <pico/bootrom.h>
#include <pico/platform/sections.h>

// SAFETY NOTES:
// flash_safe_execute_core_init must be called on the other core

DSTATUS disk_initialize(BYTE pdrv) {
	switch (pdrv) {
	case FAT_DEVICE_FLASH:
		return RES_OK;
	}
	return STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv) {
	switch (pdrv) {
	case FAT_DEVICE_FLASH:
		return RES_OK;
	}

	return STA_NOINIT;
}

// Only support a fixed section size
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
	// printf("Initiating read (sector %d count %d)\n", sector, count);
	switch (pdrv) {
	case FAT_DEVICE_FLASH:
		static_assert(FF_MIN_SS == FF_MAX_SS);
		std::size_t sector_offset = sector * FF_MAX_SS;
		std::size_t byte_count = count * FF_MAX_SS;

		if (sector_offset + byte_count > y8_fatfs_size) {
			return RES_PARERR;
		}

		std::memcpy(buff, y8_fatfs_base_cache + sector_offset, byte_count);
		return RES_OK;
	}

	return RES_PARERR;
}

#define FLASH_BLOCK_ERASE_CMD 0xd8

void __no_inline_not_in_flash_func(flash_fatfs_write)(
	const BYTE *source_buffer, std::size_t sector_flash_offset,
	std::size_t byte_count) {
	uint32_t interrupt_state = save_and_disable_interrupts();

#if PICO_RP2040
	flash_range_erase(sector_flash_offset, byte_count);
	flash_range_program(sector_flash_offset, source_buffer, byte_count);
#else
	// hacky code path that avoids reinitializing XIP
	// initializing XIP through boot2 causes issues on some boards for some
	// reason

	xip_cache_clean_all();

	rom_flash_range_erase_fn flash_range_erase_func =
		(rom_flash_range_erase_fn)rom_func_lookup_inline(
			ROM_FUNC_FLASH_RANGE_ERASE);
	rom_flash_range_program_fn flash_range_program_func =
		(rom_flash_range_program_fn)rom_func_lookup_inline(
			ROM_FUNC_FLASH_RANGE_PROGRAM);
	assert(connect_internal_flash_func && flash_range_erase_func &&
	       flash_range_program_func);

	flash_range_erase_func(sector_flash_offset, byte_count, FLASH_BLOCK_SIZE,
	                       FLASH_BLOCK_ERASE_CMD);
	flash_range_program_func(sector_flash_offset, source_buffer, byte_count);

	xip_cache_invalidate_range(sector_flash_offset, byte_count);

	__compiler_memory_barrier();
#endif

	restore_interrupts(interrupt_state);
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
	// printf("Initiating write (sector %d count %d)\n", sector, count);
	switch (pdrv) {
	case FAT_DEVICE_FLASH: {
		std::size_t sector_offset = sector * FF_MAX_SS;
		std::size_t byte_count = count * FF_MAX_SS;
		std::size_t sector_flash_offset =
			Y8_RESERVED_FIRMWARE_SIZE + sector_offset;

		if (sector_offset + byte_count > y8_fatfs_size) {
			return RES_PARERR;
		}

		arch::pico::run_blocking_command(
			arch::pico::IoThreadCommand::FLASH_LOCKOUT);

		flash_fatfs_write(buff, sector_flash_offset, byte_count);

		arch::pico::run_nofeedback_command(
			arch::pico::IoThreadCommand::FLASH_UNLOCK);

		return RES_OK;
	}
	}

	return RES_PARERR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
	// For debug:
	// printf("disk_ioctl(%d, %d, %p)\n", pdrv, cmd, buff);

	switch (pdrv) {
	case FAT_DEVICE_FLASH:
		switch (cmd) {
		case CTRL_SYNC:
			return RES_OK;
		case GET_SECTOR_COUNT:
			*static_cast<WORD *>(buff) = y8_fatfs_size / FF_MAX_SS;
			return RES_OK;
		case GET_BLOCK_SIZE: // erase block size: 4K
			static_assert(FF_MIN_SS == 4096);
			*static_cast<DWORD *>(buff) = 1;
			return RES_OK;
		default:
			return RES_PARERR;
		}
	}
	return RES_PARERR;
}
