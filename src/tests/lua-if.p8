pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- validate normal Lua syntax
if 123 == 123 then assert(true) else assert(false) end
if 123 != 123 then assert(false) else assert(true) end
if 123 == 123 then assert(true) end

-- test for the PICO-8 shorthand if syntax
-- mix some stuff around for good measure
printh("hi")
if (123 == 123) assert(true) else assert(false)
printh("hi")
if (123 != 123) assert(false) else assert(true)
printh("hi")
if (123 == 123) assert(true)
printh("hi")

function test(foo)
    if (foo == 123)
        return
    
    assert(false)
end

printh("hi")
test(123)
printh("hi")
_exit(0)
