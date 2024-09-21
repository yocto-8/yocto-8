#include <cassert>

inline void should_never_be_called() {
	assert(false && "called a stub, this should never have happened");
}