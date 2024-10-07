#pragma once

#include <cstdint>

#include <pico/multicore.h>

namespace arch::pico {

enum class IoThreadCommand : std::uint32_t {
	PUSH_FRAME,

	/// Flash lockout state.
	/// When sent from the main core, blocks execution to some code in RAM.
	/// Sends a dummy command back through the FIFO to signal readiness.
	/// Waits for a FLASH_UNLOCK command before resuming normal iteration.
	/// Other commands should never be sent in the FIFO in-between.
	FLASH_LOCKOUT,

	/// See \ref FLASH_LOCKOUT.
	FLASH_UNLOCK,
};

void core1_entry();

std::uint32_t run_blocking_command(IoThreadCommand cmd);
void run_nofeedback_command(IoThreadCommand cmd);

} // namespace arch::pico