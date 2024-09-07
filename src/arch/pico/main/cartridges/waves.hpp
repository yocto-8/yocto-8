#include <string_view>

static constexpr std::string_view cartridge =
	R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
r=64

function _update60() end
function _draw()
	cls()
	s=t()
	for y=-r,r,3 do
		for x=-r,r,2 do
			local dist=sqrt(x*x+y*y)
			z=cos(dist/40-s)*6
			pset(r+x,r+y-z,7)
		end
	end
	printh(stat(1))
end
__gfx__
)";