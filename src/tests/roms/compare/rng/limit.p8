pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- test different limit values
-- 0x1 is equivalent to not specifying the limit
-- this appears to only be a modulo over the output value

srand(0)
printh(tostr(rnd(-0x8000), true))
printh(tostr(rnd(0), true))
printh(tostr(rnd(1), true))
printh(tostr(rnd(0x10), true))
printh(tostr(rnd(0.5), true))

printh("====DONE====")
