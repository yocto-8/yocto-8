pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- FWIW, these tests also test for number parsing relatively well

function dummy_func() end

printh("empty check")
printh(tostr())

printh("nil checks")
printh(tostr(nil))
printh(tostr(nil, 0b01))
printh(tostr(nil, 0b10))
printh(tostr(nil, 0b11))

printh("table checks")
dummy_tbl = {1, 2, 3}
printh(tostr(dummy_tbl))

-- address is unique so cannot be compared but just check if it looks like an address
-- also note that for functions etc. specifying flags ALWAYS switches to address mode even if unused
-- so even 0b100 triggers the adress format

assert(sub(tostr(dummy_tbl, 0b01), 1, 9) == "table: 0x")
assert(sub(tostr(dummy_tbl, 0b10), 1, 9) == "table: 0x")
assert(sub(tostr(dummy_tbl, 0b11), 1, 9) == "table: 0x")

printh("function checks")
printh(tostr(dummy_func))
assert(sub(tostr(dummy_func, 0), 1, 12) == "function: 0x")
assert(sub(tostr(dummy_func, 0b10), 1, 12) == "function: 0x")
assert(sub(tostr(dummy_func, 0b11), 1, 12) == "function: 0x")
assert(sub(tostr(dummy_func, 0b100), 1, 12) == "function: 0x")

printh("cfunction checks")
printh(tostr(sub))

-- printh("coroutine checks")
-- printh(tostr(cocreate(dummy_func))) -- FIXME: test when implemented

printh("string checks")
printh(tostr("text"))
printh(tostr("text", 0b01))
printh(tostr("text", 0b10))
printh(tostr("text", 0b11))

printh("boolean checks")
printh(tostr(false))
printh(tostr(true))
printh(tostr(true, 0b01))
printh(tostr(true, 0b10))
printh(tostr(true, 0b11))

printh("0 check")
printh(tostr(0))
printh(tostr(0, true))
printh(tostr(0, 0b01))
printh(tostr(0, 0b10))
printh(tostr(0, 0b11))

printh("1 check")
printh(tostr(1))
printh(tostr(1, true))
printh(tostr(1, 0b01))
printh(tostr(1, 0b10))
printh(tostr(1, 0b11))

printh("-1 check")
printh(tostr(-1))
printh(tostr(-1, true))
printh(tostr(-1, 0b01))
printh(tostr(-1, 0b10))
printh(tostr(-1, 0b11))

printh("negative 0 checks")
printh(tostr(0xFFFF.FFFF))
printh(tostr(0xFFFF.FFFF, true))
printh(tostr(0xFFFF.FFFF, 0b01))
printh(tostr(0xFFFF.FFFF, 0b10))
printh(tostr(0xFFFF.FFFF, 0b11))

printh("hex fixed-point checks")
printh(tostr(0xDEAD.BEEF))
printh(tostr(0xDEAD.BEEF, true))
printh(tostr(0xDEAD.BEEF, 0b01))
printh(tostr(0xDEAD.BEEF, 0b10))
printh(tostr(0xDEAD.BEEF, 0b11))

printh("no decimal check")
printh(tostr(0xDEAD))
printh(tostr(0xDEAD, true))
printh(tostr(0xDEAD, 0b01))
printh(tostr(0xDEAD, 0b10))
printh(tostr(0xDEAD, 0b11))

printh("only decimal check")
printh(tostr(0x0.BEEF))
printh(tostr(0x0.BEEF, true))
printh(tostr(0x0.BEEF, 0b01))
printh(tostr(0x0.BEEF, 0b10))
printh(tostr(0x0.BEEF, 0b11))

printh("negative fixed-point checks")
printh(tostr(-1234.5678))
printh(tostr(-1234.5678, true))
printh(tostr(-1234.5678, 0b01))
printh(tostr(-1234.5678, 0b10))
printh(tostr(-1234.5678, 0b11))

printh("hex fixed-point checks, without all letters")
printh(tostr(0xAD.EF))
printh(tostr(0xAD.EF, true))
printh(tostr(0xAD.EF, 0b01))
printh(tostr(0xAD.EF, 0b10))
printh(tostr(0xAD.EF, 0b11))

printh("hex fixed-point checks without decimals")
printh(tostr(0xAB))
printh(tostr(0xAB, true))
printh(tostr(0xAB, 0b01))
printh(tostr(0xAB, 0b10))
printh(tostr(0xAB, 0b11))

printh("====DONE====")
_exit(0)
