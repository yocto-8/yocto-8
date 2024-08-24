pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- test different seeds

is = {0, -1, 0.5, 1, 2}

for i in all(is) do
    printh("srand("..i..")")
    srand(i)
    
    for j = 0, 10 do
        printh(j)
        printh(tostr(peek4(0x5f44), true))
        printh(tostr(peek4(0x5f48), true))
        printh(tostr(rnd(), true))
    end
end
