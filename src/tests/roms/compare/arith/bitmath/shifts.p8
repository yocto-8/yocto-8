pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
local x = 0xAAAA.BBBB

printh(tostr(x >>> 16, true))
printh(tostr(x >> 16, true))
printh(tostr(x << 16, true))
printh(tostr(x >>< 8, true))
printh(tostr(x <<> 8, true))

printh("====DONE====")