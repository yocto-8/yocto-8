#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>

extern "C" {
#ifdef Y8_USE_EXTMEM
#include "tinyalloc.hpp"

[[gnu::always_inline]]
inline void *y8_lua_realloc(void *ud, void *ptr, size_t osize, size_t nsize,
                            bool egc_recently) {
	(void)ud;

	// static int heap_use = 0;
	// heap_use += (nsize - osize);

	// printf("realloc %p %ld %ld mustnotfail=%d\n", ptr, osize, nsize,
	//        must_not_fail);
	// printf("heap: %fKiB\n", float(heap_use) / 1024.0f);

	// This is a Lua allocation function that uses standard malloc and realloc,
	// but can make use of a secondary memory pool as a fallback.

	// The secondary memory pool is used only when malloc() fails to allocate.
	// Currently, we trigger the GC to use the secondary heap as a last resort.
	// (NOTE: the performance characteristics of doing this without a smarter
	// heuristic are to be determined.)

	// The idea of letting Lua consume all of the malloc() heap is somewhat fine
	// for us, because we never allocate memory dynamically elsewhere.

	// egc counter: only retry emergency GC (EGC) if xxKiB were freed from the
	// main heap since the last EGC trigger
	static constexpr size_t egc_cooldown = 16384;

	// init the counter to the cooldown to allow one first EGC
	static size_t bytes_freed_since_egc = egc_cooldown;

	const auto &secondary_heap = *static_cast<std::span<std::byte> *>(ud);

	const bool has_extra_heap = !secondary_heap.empty();

	const auto is_ptr_on_slow_heap = [&] {
		return has_extra_heap && ptr >= secondary_heap.data() &&
		       ptr < secondary_heap.data() + secondary_heap.size();
	};

	const auto c_free = [&] {
		bytes_freed_since_egc += osize;
		free(ptr);
	};

	// Free this pointer no matter the heap it was allocated in.
	const auto auto_free = [&] {
		if (ptr == nullptr) {
			return;
		}

		if (is_ptr_on_slow_heap()) {
			ta_free(ptr);
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

		if (!egc_recently && bytes_freed_since_egc >= egc_cooldown) {
			// trigger Lua's EGC
			printf("EGC\n");
			bytes_freed_since_egc = 0;
			return nullptr;
		}

		// if (!egc_recently) {
		// 	printf("Fallback heap alloc with %d freed from main\n",
		// 	       bytes_freed_since_egc);
		// }

		if (!has_extra_heap) {
			return nullptr;
		}

		return ta_alloc(nsize);
	};

	const auto realloc_from_main_to_extra_heap = [&]() -> void * {
		if (!has_extra_heap) {
			return nullptr;
		}

		const auto new_ptr = ta_alloc(nsize);

		if (new_ptr == nullptr) {
			printf("slow heap exhausted!!\n");
			return nullptr;
		}

		memcpy(new_ptr, ptr, osize < nsize ? osize : nsize);
		c_free();
		return new_ptr;
	};

	const auto slow_realloc = [&]() -> void * {
		const auto new_ptr = auto_malloc();

		if (new_ptr == nullptr) {
			return nullptr;
		}

		memcpy(new_ptr, ptr, osize < nsize ? osize : nsize);
		auto_free();
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
	} else if (nsize < osize) {
		if (!is_ptr_on_slow_heap()) {
			const auto nptr = realloc(ptr, nsize);

			if (nptr != nullptr) {
				return nptr;
			}

			return realloc_from_main_to_extra_heap();
		}

		return slow_realloc();
	} else // nsize > osize
	{
		return slow_realloc();
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
