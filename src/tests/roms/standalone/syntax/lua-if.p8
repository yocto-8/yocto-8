pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- validate normal Lua syntax
if 123 == 123 then assert(true) else assert(false) end
if 123 != 123 then assert(false) else assert(true) end
if 123 == 123 then assert(true) end

-- test for the PICO-8 shorthand if syntax
-- mix some stuff around for good measure
if (123 == 123) assert(true) else assert(false)
if (123 != 123) assert(false) else assert(true)
if (123 == 123) assert(true)

function test(foo)
    if (foo == 123) return
    
    assert(false)
end

test(123)
printh("====DONE====")
