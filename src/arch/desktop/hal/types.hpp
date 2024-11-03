#pragma once

#include <array>
#include <cstdio>

namespace hal {

struct FileReaderContext {
	FILE *file;
	std::array<char, 256> buf;
};

} // namespace hal