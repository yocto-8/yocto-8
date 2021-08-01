#include <string_view>

static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
k={8,9,11,12}
function _update60()
    cls()
    clip(24,24,80,80)
    s=t()
    for i=1,9 do
     o=1+flr((((s*20)+i)%8)/2)
	    pal(i,k[o],0)
    end
    for y=16,96,8 do
	    d=y*0.01
	    x=64+cos(s*0.1+y*0.01)*32
	    spr(1,x-10,y+cos(s+d)*5)
	    spr(2,x,y+cos(s+d+0.1)*5)
	    spr(1,x+10,y+cos(s+d+0.2)*5)
    end
end
__gfx__
00000000011111101100001100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000222222222200002200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000333003333300003300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000440000444400004400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000550000555505505500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000666006666606606600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000777777778888888800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000088888800999999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
)";