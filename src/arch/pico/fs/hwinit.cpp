
#include <arch/pico/platform/asupico/asupico.hpp>
#include <array>
#include <cstdio>
#include <ff.h>

namespace arch::pico {

void init_flash_fatfs() {
	FRESULT res;

	res = f_mount(&platform::asupico::hw.flash_fatfs, "/flash/", 1);
	if (res != FR_OK) {
		printf("Failed to mount flash FatFS (err=%d)\n", res);

		// format flash partition
		std::array<char, FF_MAX_SS> work_buffer;
		MKFS_PARM fmt_opt = {FM_ANY, 0, 0, 0, 0};
		res =
			f_mkfs("/flash/", &fmt_opt, work_buffer.data(), work_buffer.size());

		if (res != FR_OK) {
			printf("Failed to format flash FatFS (err=%d)\n", res);
			release_abort("FatFS failed");
		}

		res = f_mount(&platform::asupico::hw.flash_fatfs, "/flash/", 1);
		if (res != FR_OK) {
			printf("Failed to mount flash FatFS after formatting (err=%d)\n",
			       res);
			release_abort("FatFS failed");
		}
	}

	f_setlabel("/flash/y8-flash");
}

} // namespace arch::pico