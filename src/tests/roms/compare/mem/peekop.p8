pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
printh("test peek operator")

poke4(0xA000, 0xDEAD.BEEF)
printh(tostr(@0xA000, true))  -- peek
printh(tostr(%0xA000, true))  -- peek2
printh(tostr($0xA000, true))  -- peek4

printh("====DONE====")