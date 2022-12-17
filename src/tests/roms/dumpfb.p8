pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
i = 0
while i != -1 do
    printh(tostr(@i))
    i = i + 1
end
printh("====DONE====")
_exit(0)
