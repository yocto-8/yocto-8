#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>

std::array<std::uint8_t, 1024> exception_buffer;
std::size_t exception_buffer_offset = 0;
std::size_t allocated_exceptions = 0;

// HACK: Must be >= the size of __cxxabiv1::__cxa_refcounted_exception
// we don't have access to the definition(?)
static constexpr std::size_t cxa_exception_size = 128;

extern "C" {

// Very basic arena allocator for C++ exceptions to avoid dynamic allocations.
// Exceptions shift the bump pointer within the preallocated buffer.
//
// This allows having multiple in-flight exceptions allocated (i.e. when dealing
// with nested exceptions, essentially).
//
// The bump pointer is reset when all exceptions are freed.
//
// This is restrictive but we are trying to keep good control on what exceptions
// we raise either way.

void *__cxa_allocate_exception(unsigned int p_size) noexcept {
	std::size_t alloc_size = p_size + cxa_exception_size;

	if (exception_buffer_offset + alloc_size >= exception_buffer.size()) {
		std::abort();
	}

	std::uint8_t *exception_storage =
		exception_buffer.data() + exception_buffer_offset;

	std::memset(exception_storage, 0, cxa_exception_size);

	exception_buffer_offset += alloc_size;
	++allocated_exceptions;

	return exception_storage + cxa_exception_size;
}

void __cxa_free_exception([[maybe_unused]] void *ptr) noexcept {
	--allocated_exceptions;

	if (allocated_exceptions == 0) {
		exception_buffer_offset = 0;
	}
}
}