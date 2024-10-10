#pragma once

#include <ff.h>

#include <emu/bufferio.hpp>

namespace hal {

struct FileReaderContext {
	union {
		emu::StringReader bios_reader = {};
		FIL fs_reader;
	} reader;

	std::array<char, 256> read_buffer;

	// hacky way to allow selecting bios read
	bool is_bios_read = false;
};

} // namespace hal