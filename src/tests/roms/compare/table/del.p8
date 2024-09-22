pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

local foo = {1, 2, 3, 4, 5}

printh(#foo)
printh(del(foo, 3))
printh(#foo)
foreach(foo, printh)

printh("====DONE====")
