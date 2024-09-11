pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

t = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

srand(0)
printh(rnd(t))
printh(rnd(t))
printh(rnd(t))

srand(0)
printh(tostr(peek4(0x5f44), true))
printh(tostr(peek4(0x5f48), true))
printh(tostr(rnd(0xFFFF), true))

printh("====DONE====")
