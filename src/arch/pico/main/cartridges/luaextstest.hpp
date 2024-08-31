#include <string_view>

static constexpr std::string_view cartridge =
	R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
i=0b10
i-=1
if (i!=0) then print("uwu") end
print(i)

function _update() end
__gfx__
)";