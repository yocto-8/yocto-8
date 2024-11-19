#include "alloc.hpp"

#ifdef Y8_USE_EXTMEM
tlsf_t global_heap;
#endif

extern "C" {
#ifdef Y8_USE_EXTMEM

// NOLINTNEXTLINE
[[gnu::used]] void *__wrap_malloc(size_t size) {
	return tlsf_malloc(global_heap, size);
}

// NOLINTNEXTLINE
[[gnu::used]] void __wrap_free(void *ptr) { tlsf_free(global_heap, ptr); }

// NOLINTNEXTLINE
[[gnu::used]] void *__wrap_realloc(void *ptr, size_t new_size) {
	return tlsf_realloc(global_heap, ptr, new_size);
}

#endif
}

void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize,
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

// void *operator new(std::size_t sz);
// void operator delete([[maybe_unused]] void *ptr) {}
// void operator delete([[maybe_unused]] void *ptr, std::size_t) noexcept {}
