#pragma once

#include "emu/emulator.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>

extern "C" {
#ifdef Y8_USE_EXTMEM
#include "tlsf.hpp"

[[gnu::section(Y8_SRAM_SECTION)]]
inline void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize,
                            [[maybe_unused]] bool egc_recently) {
	(void)ud;

	// static int heap_use = 0;
	// heap_use += (nsize - osize);

	// printf("realloc %p %ld %ld mustnotfail=%d\n", ptr, osize, nsize,
	//        must_not_fail);
	// printf("heap: %fKiB\n", float(heap_use) / 1024.0f);

	// This is a Lua allocation function that uses standard malloc and realloc,
	// but can make use of a secondary memory pool as a fallback.

	// The secondary memory pool is used only when malloc() fails to allocate.
	// Y8_USE_EGC_HEURISTIC enables triggering the GC to prefer using the backup
	// heap only as a last resort, but this seems counter-productive in general.

	// The idea of letting Lua consume all of the malloc() heap is somewhat fine
	// for us, because we never allocate memory dynamically elsewhere.

#ifdef Y8_USE_EGC_HEURISTIC
	// egc counter: only retry emergency GC (EGC) if xxKiB were freed from the
	// main heap since the last EGC trigger
	static constexpr size_t egc_cooldown = 16384;

	// init the counter to the cooldown to allow one first EGC
	static size_t bytes_freed_since_egc = egc_cooldown;
#endif

	const auto &secondary_heap = *static_cast<std::span<std::byte> *>(ud);

	const bool has_extra_heap = !secondary_heap.empty();

	const auto is_ptr_on_slow_heap = [&] {
		return has_extra_heap && ptr >= secondary_heap.data() &&
		       ptr < secondary_heap.data() + secondary_heap.size();
	};

	const auto c_free = [&] {
#ifdef Y8_USE_EGC_HEURISTIC
		bytes_freed_since_egc += osize;
#endif
		free(ptr);
	};

	// Free this pointer no matter the heap it was allocated in.
	const auto auto_free = [&] {
		if (ptr == nullptr) {
			return;
		}

		if (is_ptr_on_slow_heap()) {
			tlsf_free(emu::emulator.get_backup_heap(), ptr);
		} else {
			c_free();
		}
	};

	const auto auto_malloc = [&]() -> void * {
		void *malloc_ptr = malloc(nsize);

		if (malloc_ptr != nullptr) {
			/*if (!has_alloc_succeeded_since_egc)
			{
			    printf("recovered enough mem\n");
			}*/
			return malloc_ptr;
		}

#ifdef Y8_USE_EGC_HEURISTIC
		if (!egc_recently && bytes_freed_since_egc >= egc_cooldown) {
			// trigger Lua's EGC
			printf("EGC\n");
			bytes_freed_since_egc = 0;
			return nullptr;
		}
#endif

		if (!has_extra_heap) {
			return nullptr;
		}

		return tlsf_malloc(emu::emulator.get_backup_heap(), nsize);
	};

	const auto realloc_from_main_to_slow_heap = [&]() -> void * {
		if (!has_extra_heap) {
			return nullptr;
		}

		const auto new_ptr =
			tlsf_malloc(emu::emulator.get_backup_heap(), nsize);

		if (new_ptr == nullptr) {
			return nullptr;
		}

		memcpy(new_ptr, ptr, osize < nsize ? osize : nsize);
		c_free();
		return new_ptr;
	};

	if (nsize == 0) {
		auto_free();
		return nullptr;
	} else if (ptr == nullptr) {
		return auto_malloc();
	} else if (nsize == osize) {
		// this does happen; so let's not bother accidentally figuring out
		// if we're triggering some bad behavior un umm_realloc/newlib realloc
		return ptr;
	} else { // nsize > osize OR nsize < osize
		if (!is_ptr_on_slow_heap()) {
			// try realloc within fast heap
			const auto nptr = realloc(ptr, nsize);

			if (nptr != nullptr) {
				return nptr;
			}

			// on failure, perform slow path realloc on backup heap
			return realloc_from_main_to_slow_heap();
		}

		// orig pointer on slow heap
		return tlsf_realloc(emu::emulator.get_backup_heap(), ptr, nsize);
	}
}
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
