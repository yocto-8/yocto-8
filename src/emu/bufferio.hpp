#pragma once

#include "coredefs.hpp"
#include <hal/hal.hpp>

#include <string_view>

namespace emu {

/// @brief Wraps a reader per the HAL definition
struct Reader {
	hal::ReaderCallback *callback;
	void *user_data;

	const char *operator()(std::size_t *size) {
		return callback(user_data, size);
	}
};

/// @brief Unbuffered reader which forwards an entire `std::string_view` to Lua.
struct StringReader {
	std::string_view str;
	bool consumed = false;

	static const char *reader_callback(void *self, std::size_t *size) {
		auto &reader = *static_cast<StringReader *>(self);

		if (reader.consumed) {
			*size = 0;
			return nullptr;
		}

		reader.consumed = true;
		*size = reader.str.size();
		return reader.str.data();
	}

	Reader get_reader() { return {reader_callback, this}; }
};

/// @brief Cart importer, which can inject a header into the buffer and inline
/// included files through @ref SourceBufferReader
struct CartImporterReader {
	StringReader header_reader;
	Reader cart_reader;

	enum class ActiveReader { HEADER, CART };
	ActiveReader active_reader = ActiveReader::HEADER;

	static const char *reader_callback(void *self, std::size_t *size) {
		auto &importer = *static_cast<CartImporterReader *>(self);

		switch (importer.active_reader) {
		case ActiveReader::HEADER: {
			// callback will always end in one time
			importer.active_reader = ActiveReader::CART;
			return StringReader::reader_callback(&importer.header_reader, size);
		}
		case ActiveReader::CART: {
			return importer.cart_reader(size);
		}
		}

		release_abort("Unexpected cart reader state");
		return nullptr;
	}
};

} // namespace emu
