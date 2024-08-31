#pragma once

#include <cstdint>

#include <devices/image.hpp>

namespace hal {

/// @brief Reads the button state as a bitfield in the form of a bitfield.
/// @returns In the returned bitfield, the Nth bit matches the button in the
/// io::Button namespace. Other bits should be assumed to be reserved.
[[nodiscard]] std::uint16_t update_button_state();

/// @brief Presents a new frame. May or may not perform double-buffering
/// internally.
void present_frame();

/// @brief Reset the time measurement timer use in measure_time_us()
void reset_timer();

/// @brief Measure and return the current time in microseconds.
/// @warning This may overflow after around 584542 years. I won't be the
/// maintainer by then, so not my problem.
std::uint64_t measure_time_us();

/// @brief Wait for approximately @p time microseconds.
void delay_time_us(std::uint64_t time);

/// @brief Load a new RGB palette to use from the next presented frame onwards.
void load_rgb_palette(std::span<std::uint32_t, 32> new_palette);

/// @brief Gets the default/precalibrated color for this platform.
std::span<const std::uint32_t, 32> get_default_palette();

} // namespace hal