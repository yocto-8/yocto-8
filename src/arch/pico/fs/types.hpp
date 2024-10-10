#pragma once

#include <cstddef>
#include <hardware/regs/addressmap.h>
#include <pico.h>

enum FatDeviceID {
	FAT_DEVICE_FLASH = 0,
	FAT_DEVICE_SD = 1,
};

extern "C" {
extern char __flash_binary_start, __flash_binary_end;
}

inline const std::size_t y8_firmware_size =
	&__flash_binary_end - &__flash_binary_start;

inline const std::size_t y8_fatfs_size =
	PICO_FLASH_SIZE_BYTES - Y8_RESERVED_FIRMWARE_SIZE;

inline char *y8_fatfs_base_nocache = reinterpret_cast<char *>(
	XIP_NOCACHE_NOALLOC_BASE + Y8_RESERVED_FIRMWARE_SIZE);

static_assert(Y8_RESERVED_FIRMWARE_SIZE % 4096 == 0, "Firmware size must land on 4K page boundary!");

inline char *y8_fatfs_base_cache =
	reinterpret_cast<char *>(XIP_BASE + Y8_RESERVED_FIRMWARE_SIZE);