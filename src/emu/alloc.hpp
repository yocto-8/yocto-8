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

#ifdef Y8_USE_EXTMEM
void *__wrap_malloc(size_t size);
void __wrap_free(void *ptr);
void *__wrap_realloc(void *ptr, size_t new_size);
#endif

inline void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize,
                            [[maybe_unused]] bool must_not_fail) {
	(void)ud;

	if (nsize == 0) {
#ifdef Y8_USE_EXTMEM
		__wrap_free(ptr);
#else
		free(ptr);
#endif
		return nullptr;
	} else if (ptr == nullptr) {
#ifdef Y8_USE_EXTMEM
		return __wrap_malloc(nsize);
#else
		return malloc(nsize);
#endif
	} else if (nsize == osize) {
		return ptr;
	} else // (nsize < osize) || (nsize > osize)
	{
#ifdef Y8_USE_EXTMEM
		return __wrap_realloc(ptr, nsize);
#else
		return realloc(ptr, nsize);
#endif
	}
}
}
