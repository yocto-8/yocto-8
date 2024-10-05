pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

-- test examples from wiki

printh(tonum('12345'))
printh(tonum('-12345.67'))
printh(tonum('-1.23456789e4'))
printh(tonum('0x0f'))
printh(tonum('0x0f.abc'))
printh(tonum('0b1001'))
printh(tonum('32767'))
printh(tonum('99999'))
printh(tonum('xyz'))
printh(tonum("ff",       0x1))
printh(tonum("1146880",  0x2))
printh(tonum("1234abcd", 0x3))
printh(tonum("xyz",      0x4))

printh("====DONE====")
