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
}

void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize,
                     [[maybe_unused]] bool must_not_fail);
