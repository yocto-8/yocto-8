#pragma once

#include <hal/hal.hpp>

#include <array>
#include <string_view>

namespace emu {

/// @brief Buffered reader which reads and concatenates sources in a way that
/// inlines `#include` statements contained within it.
///
/// `#include` statements result in a call into the relevant `hal::` APIs.
///
/// The top-level reader is defined separately and is user-provided.
class SourceBufferReader {
	public:
	SourceBufferReader(hal::ReaderCallback *top_level_reader,
	                   void *top_level_ud)
		: top_level_reader(top_level_reader), top_level_ud(top_level_ud),
		  buffer_idx(0) {}

	static const char *reader_callback(void *self, std::size_t *size);

	private:
	hal::ReaderCallback *top_level_reader;
	void *top_level_ud;

	/// @brief Stack of buffers, where `buffer_idx-1` points to the topmost
	/// element (inclusive). Each `string_view` represents what remains of the
	/// buffer for a given file. It is a stack (of limited size) in order to
	/// handle arbitrary include depth.
	std::array<hal::FileReaderContext, 8> include_buf_stack;

	/// @brief If `== 0`, the active buffer is the `top_level_reader`.
	/// Otherwise, `buffer_idx - 1` refers to the index of the buffer that
	/// should be next read in @ref include_buf_stack.
	std::size_t buffer_idx;
};

/// @brief Unbuffered reader which forwards an entire `std::string_view` to Lua.
struct StringReader {
	std::string_view str;
	bool consumed = false;

	static const char *reader_callback(void *self, std::size_t *size);
};

/// @brief Cart importer, which can inject a header into the buffer and inline
/// included files through @ref SourceBufferReader
struct CartImporterReader {
	StringReader header_reader;
	SourceBufferReader cart_reader;

	enum class ActiveReader { HEADER, CART };
	ActiveReader active_reader = ActiveReader::HEADER;

	static const char *reader_callback(void *self, std::size_t *size);
};

} // namespace emu
