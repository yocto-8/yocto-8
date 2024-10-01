#pragma once

#include <array>
#include <cstdio>

namespace hal {

struct FileReaderContext {
	FILE *file;
	std::array<char, 4096> buf;
};

} // namespace hal