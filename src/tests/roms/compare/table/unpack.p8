pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

function foo(a, b, c, d)
    printh(tostr(a))
    printh(tostr(b))
    printh(tostr(c))
    printh(tostr(d))
end

foo(unpack({123, nil, 456}))

printh("====DONE====")
