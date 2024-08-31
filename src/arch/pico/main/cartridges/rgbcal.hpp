#include <string_view>

static constexpr std::string_view cartridge =
	R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
entry = 0
ch = 0
delay = 3

function _update()
    if delay <= 3 then
        delay += 1
        return
    end

    if btn(2) then entry -= 1; delay = 0 end
    if btn(3) then entry += 1; delay = 0 end

    if btn(0) then ch -= 1; delay = 0 end
    if btn(1) then ch += 1; delay = 0 end

    entry = abs(entry % 16)
    ch = abs(ch % 3)

    r,g,b=_rgbpal(entry)
    cols={r,g,b}
    if btn(4) then
        cols[ch+1] += 1
        delay = 0
    end
    if btn(5) then
        cols[ch+1] -= 1
        delay = 0
    end

    _rgbpal(entry, cols[1], cols[2], cols[3])
end

function _draw()
    cls(0)

    for i=0,15 do
        x=26
        y=i*8

        if i == entry then x += 4 end

        r,g,b=_rgbpal(i)
        print("#"..i..": ", x, y, 7)
        x += 4*5
        print(r, x, y, (ch == 0) and 7 or 5)
        x += 4*4
        print(g, x, y, (ch == 1) and 7 or 5)
        x += 4*4
        print(b, x, y, (ch == 2) and 7 or 5)

        rectfill(0,y,23,y+7,i)
        rectfill(103,y,127,y+7,i)
    end
end
__gfx__)";