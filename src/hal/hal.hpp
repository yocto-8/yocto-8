#pragma once

#include <cstdint>
#include <gsl/gsl>

#include <video/framebufferview.hpp>

namespace hal
{

/// @brief Reads the button state as a bitfield in the form of a bitfield.
/// @returns In the returned bitfield, the Nth bit matches the button in the io::Button namespace.
/// Other bits should be assumed to be reserved.
[[nodiscard]] std::uint16_t update_button_state();

/// @brief Presents a new frame. May or may not perform double-buffering internally.
void present_frame(video::FramebufferView view);

}