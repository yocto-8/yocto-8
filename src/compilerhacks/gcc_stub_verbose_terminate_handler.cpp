#include "gcc_stub_common.hpp"

namespace __gnu_cxx {
[[gnu::cold]] void __verbose_terminate_handler() {
	for (;;)
		;
}
} // namespace __gnu_cxx