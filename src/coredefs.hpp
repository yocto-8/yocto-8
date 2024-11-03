#pragma once

#include "luaconf.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace y8 {

using byte = std::byte;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using Number = LuaFix16;
using RawFix16 = fix16_t;
using PicoAddr = u16;

// FIXME: this should print to stderr, but this results in code bloat on pico
// builds. why is that?

#define release_abort(reason)                                                  \
	printf("Runtime BUG (%s:%d): %s\n", __FILE__, __LINE__, reason);           \
	std::abort();

#define debug_assert(x) assert(x)

#define release_assert(x)                                                      \
	if (!(x)) [[unlikely]] {                                                   \
		release_abort(#x)                                                      \
	}

} // namespace y8