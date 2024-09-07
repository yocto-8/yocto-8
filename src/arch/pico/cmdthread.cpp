#include "cmdthread.hpp"

#include <pico/multicore.h>
#include <pico/platform.h>

#include <devices/image.hpp>
#include <devices/screenpalette.hpp>
#include <emu/emulator.hpp>
#include <hardwarestate.hpp>
#include <platform/platform.hpp>

#include <cstdio>

namespace arch::pico {

inline void multicore_fifo_write_assume_nonfull(std::uint32_t value) {
	assert(sio_hw->multicore_fifo_wready());
	sio_hw->fifo_wr = value;
	__sev(); // fire event
}

void __scratch_x("core1_irq") core1_sio_irq() {
	const auto command = IoThreadCommand(sio_hw->fifo_rd);

	switch (command) {
	case IoThreadCommand::PUSH_FRAME: {
		platform::present_frame([] {
			// copy done - we can update in background now
			multicore_fifo_write_assume_nonfull(0);
		});

		break;
	}
	}

	multicore_fifo_clear_irq();
}

void core1_entry() {
	irq_set_exclusive_handler(SIO_FIFO_IRQ_NUM(1), core1_sio_irq);
	irq_set_enabled(SIO_FIFO_IRQ_NUM(1), true);

	platform::local_core_init();

	// TODO: look into deep sleep bits to save power here, if viable
	for (;;) {
		__wfi();
	}
}

std::uint32_t run_blocking_command(IoThreadCommand cmd) {
	multicore_fifo_push_blocking(std::uint32_t(cmd));
	return multicore_fifo_pop_blocking();
}

} // namespace arch::pico