#include <string_view>

static constexpr std::string_view cartridge =
	R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
i=0
function _update()
    c=i
    dir=1
    if btn(0) then dir=-1 end
    for y=0,127 do
        c=c+dir
        for x=0,127 do
            pset(x,y,(c+x)/16)
        end
	end
	i=i+1
end
__gfx__)";