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

[[gnu::always_inline]]
inline void multicore_fifo_write_assume_nonfull(std::uint32_t value) {
	assert(sio_hw->multicore_fifo_wready());
	sio_hw->fifo_wr = value;
	__sev(); // fire event
}

void __scratch_x("core1_irq") core1_sio_irq() {
	// eat all the FIFO requests we can then clear IRQ (and return to sleep)
	while ((sio_hw->fifo_st & SIO_FIFO_ST_VLD_BITS) != 0) {
		const auto command = IoThreadCommand(sio_hw->fifo_rd);

		switch (command) {
		case IoThreadCommand::PUSH_FRAME: {
			platform::present_frame([] {
				// copy done - we can update in background now
				multicore_fifo_write_assume_nonfull(0);
			});

			break;
		}

		case IoThreadCommand::FLASH_LOCKOUT: {
			multicore_fifo_write_assume_nonfull(0);

			uint32_t interrupt_state = save_and_disable_interrupts();

			// TODO: a spinlock is very dumb here but
			// 1. what else?
			// 2. it only happens on flash writes, which are _kind of_
			// uncommon...
			while ((sio_hw->fifo_st & SIO_FIFO_ST_VLD_BITS) == 0)
				;
			[[maybe_unused]] const std::uint32_t out =
				sio_hw->fifo_rd; // trigger read

			restore_interrupts(interrupt_state);

			break;
		}

		case IoThreadCommand::FLASH_UNLOCK: {
			release_abort(
				"FLASH_UNLOCK should have been caught by FLASH_LOCKOUT");
		}
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

void run_nofeedback_command(IoThreadCommand cmd) {
	multicore_fifo_push_blocking(std::uint32_t(cmd));
}

} // namespace arch::pico