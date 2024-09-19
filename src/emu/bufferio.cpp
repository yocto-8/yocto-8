#include "bufferio.hpp"
#include "hal/hal.hpp"
#include "hal/types.hpp"
#include <cassert>

namespace emu {

const char *StringReader::reader_callback(void *self, std::size_t *size) {
	auto &reader = *static_cast<StringReader *>(self);

	if (reader.consumed) {
		*size = 0;
		return nullptr;
	}

	reader.consumed = true;
	*size = reader.str.size();
	return reader.str.data();
}

const char *SourceBufferReader::reader_callback(void *self, std::size_t *size) {
	auto &source_reader = *static_cast<SourceBufferReader *>(self);

	const char *buf;

	if (source_reader.buffer_idx == 0) {
		buf = source_reader.top_level_reader(source_reader.top_level_ud, size);
	} else {
		assert(false);
		hal::FileReaderContext &active_file_reader =
			source_reader.include_buf_stack[source_reader.buffer_idx];
	}

	// const char *buf = hal::fs_read_buffer(&active_file_reader, size);

	// FIXME: add include injection functionality. means to handle EOF etc. the
	// callee must not see a null/0-sized buffer if we are not fully done
	// emitting.
	//
	// also this would be a massive fucking pain for large input buffers? like
	// how to split up everything?

	return buf;

	// TODO: NDEBUG assert
	// assert(source_reader.buffer_idx + 1 <
	//        source_reader.include_buf_stack.size());
}

const char *CartImporterReader::reader_callback(void *self, std::size_t *size) {
	auto &importer = *static_cast<CartImporterReader *>(self);

	switch (importer.active_reader) {
	case ActiveReader::HEADER: {
		importer.active_reader =
			ActiveReader::CART; // callback will always end in one time
		return StringReader::reader_callback(&importer.header_reader, size);
	}
	case ActiveReader::CART: {
		return SourceBufferReader::reader_callback(&importer.cart_reader, size);
	}
	}
}

} // namespace emu