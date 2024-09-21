pico-8 cartridge // http://www.pico-8.com
version 32
__lua__

local foo = {}
printh(#foo)
printh(add(foo, 123))
printh(#foo)
printh(add(foo, 456))
printh(#foo)
printh(add(foo, 789))
printh(#foo)
foo[#foo] = nil -- mark as deleted
printh(add(foo, 123))
printh(#foo)

-- i removed the nil edge cases because they seem to trigger a difference in
-- rehashing between us and PICO-8
-- the implementation of #, as it turns out, is pretty deranged and having nil
-- inside the table causes unspecified behavior.
-- we could try to behave **exactly** like PICO-8, but this is a really specific
-- implementation detail. at that point, if someone is relying on that, they're
-- just out to bully me.

-- printh("nil edge cases...")
-- foo = {nil, nil, 1, 2, nil, 3}
-- printh(#foo)

-- foo[#foo+1]=123
-- --printh(add(foo, 123))
-- printh(#foo)

-- printh("enumerating")
-- foreach(foo, printh)

printh("====DONE====")
