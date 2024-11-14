#pragma once

#include <cstddef>
#include <cstdint>
#include <lua.hpp>
#include <string_view>

#include <devices/image.hpp>

namespace hal {

struct ButtonState {
	std::uint8_t held_key_mask;
	std::uint8_t pressed_key_mask;
};

/// @brief Reads the button state as a bitfield in the form of a bitfield.
/// @returns In the returned bitfield, the Nth bit matches the button in the
/// io::Button namespace. Other bits should be assumed to be reserved.
[[nodiscard]] ButtonState update_button_state();

/// @brief Presents a new frame. May or may not perform double-buffering
/// internally.
void present_frame();

/// @brief Reset the time measurement timer use in measure_time_us()
void reset_timer();

/// @brief Measure and return the current time in microseconds.
/// @warning This may overflow after around 584542 years. I won't be the
/// maintainer by then, so not my problem.
[[nodiscard]] std::uint64_t measure_time_us();

/// @brief Function called just prior to sleeping before the next tick, usually
/// to do some hardware housekeeping.
void post_frame_hooks();

/// @brief Wait for approximately @p time microseconds.
void delay_time_us(std::uint64_t time);

/// @brief Load a new RGB palette to use from the next presented frame onwards.
void load_rgb_palette(std::span<std::uint32_t, 32> new_palette);

/// @brief Gets the default/precalibrated color for this platform.
[[nodiscard]] std::span<const std::uint32_t, 32> get_default_palette();

/// @brief Reads user input from the standard input into the provided buffer, if
/// supported. Non-blocking. Used for debugging; exceeding buffer size does not
/// need to be correctly handled/notified (but won't cause UB).
/// @returns Subspan representing the parsed string within the `target_buffer`.
/// Empty on no input or on an empty input.
[[nodiscard]] std::span<char> read_repl(std::span<char> target_buffer);

/// @brief Platform-specific struct.
struct FileReaderContext;

enum class FileOpenStatus { SUCCESS, FAIL };

/// @brief Sets the active working directory to the given path. Following this,
/// files can be opened
void fs_set_working_directory(std::string_view path);

/// @brief Opens a file with the given path in the filesystem in a blocking
/// fashion. Initializes the `out` context.
/// The path is resolved according to the current working directory, unless an
/// absolute path is given.
/// @returns FileOpenStatus::SUCCESS is the file is readable, FAIL otherwise.
[[nodiscard]] FileOpenStatus fs_create_open_context(std::string_view path,
                                                    FileReaderContext &out);

/// @brief Closes/destroys a file read context. Should not be called if file
/// open had failed with any non-SUCCESS status. No further reader callback
/// should ever be issued after destruction.
void fs_destroy_open_context(FileReaderContext &ctx);

/// @brief Callback for filesystem file reading. Uses semantics similar to that
/// of `lua_Reader`. Note that the caller is not in control of buffering. The
/// backend can return any number of bytes, but a `*size` of 0 is exclusively
/// used to signal EOF to the caller.
///
/// The returned buffer is expected to survive until either:
///
/// - `fs_destroy_open_context` is called for the associated context
/// - **or** `fs_read_buffer` is called over the same context again
///
/// @arg context Opaque pointer to the @ref FileReaderContext
/// @arg size Pointer to the `size_t` that will receive the size of the buffer.
/// The callback should write 0 to it when the file is done reading and all
/// chunks were read.
/// @returns Pointer to a buffer.
using ReaderCallback = const char *(void *context, std::size_t *size);

/// @brief Reader callback for a file, where `context` is a pointer to an object
/// of type @ref FileReaderContext as previously returned by @ref
/// fs_create_open_context. Compatible with @ref FileChunkReaderCallback
[[nodiscard]] const char *fs_read_buffer(void *context, std::size_t *size);

struct FileInfo {
	enum class Kind { DIRECTORY, FILE };

	Kind kind;
	std::string_view name;
};

/// @brief Callback called to describe a single file or directory when
/// enumerating files in a directory. When it returns false, enumeration may be
/// aborted.
using DirectoryListCallback = bool(void *ud, const FileInfo &info);

enum class DirectoryListStatus { SUCCESS, FAIL };

/// @brief Enumerate files in a directory, invoking the callback for each
/// element. See @ref DirectoryListCallback for more details.
/// @arg path is the path of the directory to enumerate (potentially relative to
/// the active directory). When `nullptr` (the default), refer to the active
/// directory.
DirectoryListStatus fs_list_directory(DirectoryListCallback *callback, void *ud,
                                      const char *path = nullptr);

/// @brief Attempt to get a true random seed.
[[nodiscard]] std::uint32_t get_unique_seed();

} // namespace hal
