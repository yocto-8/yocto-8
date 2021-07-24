#include "emu/emulator.hpp"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <extmem/spiram.hpp>
#include <extmem/cachedinterface.hpp>
#include <video/ssd1351.hpp>
#include <io/button.hpp>
#include <p8/parser.hpp>

#include "hardwarestate.hpp"

namespace pico = arch::pico;

// FIXME: __lua__ fucks up if a later chunk is not present
/*
static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
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
*/
/*
static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- 1-d cellular automata demo
-- by zep
-- ref: wikipedia.org/wiki/cellular_automaton

cls()
l=0 -- line count

--uncomment for kaleidoscope
--poke(0x5f2c,7)

-- starting rule set
r={[0]=0,1,0,1,1,0,0,1}

bitch = {1,2,4}

function _update()

	l=l+1
	-- change rule every 16 lines
	-- (or when ❎ is pressed)
	--if (l%16==0 or btn(❎)) then
    if (l%16==0 or btn(0)) then
		for i=1,7 do
			--r[i]=(r[i]+1)%3
			r[i]=flr(rnd(2.3))
			print(r[i])
		end
	end
	
	
	-- if the line is blank, add
	-- something to get it started
	found = false
	for x=0,127 do
		--if (pget(x,127)>0) found=true
        if (pget(x,127)>0) then found=true end
	end
	
	if (not found) then
		pset(63,127,7)
	end

--end

--function _draw()
	-- scroll
	memcpy(0x6000,0x6040,0x1fc0)
	
	for x=0,127
	 do n=0 
	 for b=0,2 do
	  if (pget(x-1+b,126)>0)
	  then
      -- FIXME: 2^n borked
	   n = n + bitch[b+1] -- 1,2,4
	  end
	 end
	 pset(x,127,r[n]*7)
	end
end
__gfx__
)";*/
/*static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
r=64

function _update()
	cls()
	s=t()
		for y=-r,r,3 do
			for x=-r,r,2 do
				local dist=sqrt(x*x+y*y)
				z=cos(dist/40-s)*6
				pset(r+x,r+y-z,7)
		end
	end
end
__gfx__
)";*/
static constexpr std::string_view cartridge = R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
i=0
i+=1
if (i!=0) then print("uwu") end
print(i)

function _update() end
__gfx__
)";

int main()
{
    pico::initialize_hardware();

    p8::Parser parser(cartridge);

    while (parser.parse_line())
        ;
    
    emu::emulator.run();

    /*for (;;)
    {
        {
            const auto time_start = get_absolute_time();
            emu::emulator.hook_update();
            const auto time_end = get_absolute_time();
            printf("_update() took %fms\n", absolute_time_diff_us(time_start, time_end) / 1000.0f);
        }

        {
            const auto time_start = get_absolute_time();
            pico::hw.ssd1351.update_frame();
            const auto time_end = get_absolute_time();
            printf("update_frame() took %fms\n", absolute_time_diff_us(time_start, time_end) / 1000.0f);
        }
    }*/
}