#include "gcc_stub_common.hpp"

extern "C" {
void __gxx_personality_v0() { should_never_be_called(); }
}
