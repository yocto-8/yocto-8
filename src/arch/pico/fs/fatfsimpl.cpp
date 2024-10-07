#include <ff.h>

// diskio.h include must be after ff.h
#include <diskio.h>

#include <cstdio>

enum DeviceID {
	FLASH = 0,
	SD = 1,
};

DSTATUS disk_initialize(BYTE pdrv) {
	switch (pdrv) {
	case FLASH:
		return RES_OK;
	}
	return STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv) {
	switch (pdrv) {
	case FLASH:
		return RES_OK;
	}

	return STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
	printf("TODO: read\n");
	switch (pdrv) {
	case FLASH:
		return RES_ERROR;
	}

	return RES_PARERR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
	printf("TODO: write\n");
	switch (pdrv) {
	case FLASH:
		return RES_ERROR;
	}

	return RES_PARERR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) { return RES_PARERR; }