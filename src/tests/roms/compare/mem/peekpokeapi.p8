pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
printh("test wraparound behavior w/ different peek granularities")

poke4(0xFFFF, 0xDEAD.BEEF)
a, b, c, d = peek(0xFFFF, 4)
printh(tostr(a, true))
printh(tostr(b, true))
printh(tostr(c, true))
printh(tostr(d, true))
printh(tostr(peek4(0xFFFF), true))
printh(tostr(peek2(0xFFFF), true))
printh(tostr(peek(0xFFFF), true))
printh(tostr(peek4(0x0000), true))

-- test variadic poke
poke(0xFFFF, 0x1234.5678)
a, b, c, d = peek(0xFFFF, 4)
printh(tostr(a, true))
printh(tostr(b, true))
printh(tostr(c, true))
printh(tostr(d, true))

-- TODO: test memcpy/memset wraparound behavior
-- TODO: check memcpy/memset wraparound behavior
-- TODO: implement peek operators

printh("====DONE====")