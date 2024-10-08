pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
local y = 0xBBBB.CCCC
local s = 4

local x = 0xAAAA.BBBB
x |= y
printh(tostr(x, true))

local x = 0xAAAA.BBBB
x ^^= y
printh(tostr(x, true))

local x = 0xAAAA.BBBB
x &= y
printh(tostr(x, true))

local x = 0xAAAA.BBBB
x <<= s
printh(tostr(x, true))

local x = 0xAAAA.BBBB
x >>= s
printh(tostr(x, true))

local x = 0xAAAA.BBBB
x <<>= s
printh(tostr(x, true))

local x = 0xAAAA.BBBB
x >><= s
printh(tostr(x, true))

-- test integer division for good measure
local x = 5
x \= 5
print(tostr(x, true))

printh("====DONE====")