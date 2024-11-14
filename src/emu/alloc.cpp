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
