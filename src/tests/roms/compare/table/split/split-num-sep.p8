pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

test_str = "0x1.1,b,c,2.35,"

printh("num numeric split edge case")
foreach(split(test_str, -1), printh)

printh("num numeric split edge case")
foreach(split(test_str, 1), printh)

printh("num numeric split edge case")
foreach(split(test_str, 2), printh)

printh("====DONE====")
