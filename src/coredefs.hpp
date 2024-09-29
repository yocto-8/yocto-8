#pragma once

#include "luaconf.h"
#include <cstddef>
#include <cstdint>

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

#define debug_assert(x) assert(x)
#define release_assert(x) assert(x)

} // namespace y8