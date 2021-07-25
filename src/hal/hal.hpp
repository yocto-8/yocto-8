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
void present_frame();

/// @brief Reset the time measurement timer use in measure_time_us()
void reset_timer();

/// @brief Measure and return the current time in microseconds.
/// @warning This may overflow after around 584542 years. I won't be the maintainer by then, so not my problem.
std::uint64_t measure_time_us();

}