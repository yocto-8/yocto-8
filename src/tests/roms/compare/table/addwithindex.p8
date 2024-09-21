pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

local foo = {1, 2, 3, 4}
printh(add(foo, 123, 2))
foreach(foo, printh)

printh("====DONE====")
