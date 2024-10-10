#pragma once

#include <pico/unique_id.h>

namespace arch::pico {

extern "C" {
extern char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
}

void init_usb_device();

} // namespace arch::pico