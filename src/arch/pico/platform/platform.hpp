#pragma once

namespace arch::pico::platform {

/// @brief Executed immediately on main(), after static initialization.
void init_hardware();

using FrameCopiedCallback = void();

/// Called when the frame should be presented.
///
/// If the platform so desires, it can be copy the frame data
/// (Framebuffer and ScreenPalette) before actually presenting the frame.
///
/// The platform MUST call \p callback ONCE to signal that the main thread
/// should unblock. Should it fail to do so, the main core will hang.
///
/// Bear in mind that this is executed on the secondary core.
/// As a result, the primary core may be modifying the frame data concurrently
/// after calling \p callback.
/// This could cause tearing, which may be more or less pronounced depending on
/// how fast you can transfer data.
///
/// Why is it done this way instead of splitting into a `copy_frame` function?
/// Because usage of a callback allows optionally copying stuff to the stack
/// instead of being forced through "slow" (difficult to optimize) global state.
void present_frame(FrameCopiedCallback *callback = nullptr);

} // namespace arch::pico::platform