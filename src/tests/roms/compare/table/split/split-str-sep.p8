pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

test_str = "a,b,c"

printh("simple default params")
foreach(split(test_str), printh)

printh("simple explicit default and no parse int")
foreach(split(test_str, ",", false), printh)

printh("simple explicit parse int")
foreach(split(test_str, ",", true), printh)

test_str = "0x1.1,b,c,2.35,"

printh("num default params")
foreach(split(test_str), printh)

printh("num explicit default and no parse int")
foreach(split(test_str, ",", false), printh)

printh("num explicit parse int")
foreach(split(test_str, ",", true), printh)

printh("====DONE====")
