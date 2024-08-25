pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
assert(cos(0) == 1.0)
assert(cos(-1) == 1.0)
assert(cos(1) == 1.0)
assert(cos(0.5) == -1.0)
assert(cos(0.75) >= -0.0001 and cos(0.75) <= 0.0001)
assert(cos(0.9) >= 0.8090 and cos(0.9) <= 0.8092)
assert(cos(-0.5) >= -1.0001 and cos(-0.5) <= -0.9999)
printh(cos(-0.75))
assert(cos(-0.75) >= -0.0001 and cos(-0.75) <= 0.0001)
assert(cos(-0.9) >= 0.8088 and cos(-0.9) <= 0.8090)
assert(cos(100) == 1.0)
printh("====DONE====")
