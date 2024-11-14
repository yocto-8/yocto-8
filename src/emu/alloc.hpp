#pragma once

#include "emu/tlsf.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>

#ifdef Y8_USE_EXTMEM
extern tlsf_t global_heap;
#endif

extern "C" {

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
}
