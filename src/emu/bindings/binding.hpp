#pragma once

// Forward declare so we do not require including lua.h in every binding file
typedef struct lua_State lua_State;

/// Function attributes for functions part of the core API, which don't
/// represent much code size, but should be consistently fast
#define Y8_FAST_BINDING __attribute__((section(Y8_SRAM_SECTION), hot))