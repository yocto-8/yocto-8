pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

printh(ord("hello world"))

printh("multi ret")
a, b = ord("hello world", 2)
printh(a)
printh(b)

printh("start index oob")
a, b = ord("hello world", 0, 1)
printh(a)
printh(b)

printh("too high start index, should not return")
printh(ord("hello world", 100, 1))

printh("length of 0")
printh(ord("hello world", 1, 0))

printh("too high length, nil oob")
a, b = ord("hello world", 11, 2)
printh(a)
printh(b)

printh("====DONE====")