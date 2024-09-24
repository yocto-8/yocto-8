pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
local x = 0xAAAA.BBBB
local y = 0xBBBB.CCCC

printh(tostr(x | y, true))
printh(tostr(x ^^ y, true))
printh(tostr(x & y, true))
printh(tostr(~x, true))

printh("====DONE====")