#include <string_view>

static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- drippy
-- by zep

--rectfill(0,0,127,127,1)
cls(0)
x=64 y=64 c=8

function _update()
	pset(x,y,c)
	
	if (btn(0)) then x=x-1 end
	if (btn(1)) then x=x+1 end
	if (btn(2)) then y=y-1 end
	if (btn(3)) then y=y+1 end
	
	c=c+1/8
	if (c >= 16) then c = 8 end
	
	-- increase this number for
	-- extra drippyness
	for i=1,800 do 
	
	-- choose a random pixel
	local x2 = rnd(128)
	local y2 = rnd(128)
	local col = pget(x2,y2)
	
	--drip down if it is colourful
	if (col > 1) then
		pset(x2,y2+1,col) 
	end
	end
	
end
__gfx__
)";