#include <cstdio>

namespace __gnu_cxx {
[[gnu::cold]] void __verbose_terminate_handler() {
	printf("Crash: terminate() called!\n");
	for (;;)
		;
}
} // namespace __gnu_cxx