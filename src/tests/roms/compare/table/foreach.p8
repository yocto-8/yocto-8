pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

local foo = {nil, nil, 1, 1, 2, nil, 3}
foreach(foo, printh)

foo[1] = 4
foreach(foo, printh)

printh("with keys")
foo = {1, 2, 3, a=123, b=456}
foreach(foo, printh)

printh("with dupes")
s = "foo"
foo = {s, s, s}
foreach(foo, printh)

printh("completely deranged and evil deletion")
foo = {"a", "b", "c"}
local evil_evil_evil = function(x)
    del(foo, x)
    printh(x)
end
foreach(foo, evil_evil_evil)

printh("syntax edge cases")
printh(foreach())
printh(foreach(nil))

printh("====DONE====")
