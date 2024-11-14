#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#ifdef Y8_USE_EXTMEM

// define in .cpp
void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize,
                     [[maybe_unused]] bool egc_recently);
#else
inline void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize,
                            [[maybe_unused]] bool must_not_fail) {
	(void)ud;

	if (nsize == 0) {
		free(ptr);
		return nullptr;
	} else if (ptr == nullptr) {
		return malloc(nsize);
	} else if (nsize == osize) {
		return ptr;
	} else // (nsize < osize) || (nsize > osize)
	{
		return realloc(ptr, nsize);
	}
}
#endif
}
