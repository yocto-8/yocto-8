#pragma once

#include <emu/bufferio.hpp>

namespace hal {

struct FileReaderContext {
	union {
		emu::StringReader bios_reader = {};
	} reader;

	// hacky way to allow selecting bios read
	bool is_bios_read = false;
};

} // namespace hal