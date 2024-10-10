#include "hwinit.hpp"

#include <pico/unique_id.h>
#include <tusb.h>

namespace arch::pico {
char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

void init_usb_device() {
	pico_get_unique_board_id_string(serial, sizeof(serial));
	tud_init(BOARD_TUD_RHPORT);
}
} // namespace arch::pico