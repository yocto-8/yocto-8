#include "cachedinterface.hpp"

namespace extmem
{

std::uintptr_t active_cache_page_index = 0;
std::array<char, cache_size> cache = {0};

}