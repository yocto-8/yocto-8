pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
function assert_isclose(x, y)
    -- TODO: the epsilon here is rather high
    -- work towards reducing it to better match PICO-8
    -- presumably a LUT solution could work and might need less memory than the stock libfixmath LUT?
    eps = 0.01
    if x >= y - eps and x <= y + eps then
        printh(y)  -- print expected value so diff doesn't fail
    else
        printh(x)  -- print the incorrect obtained value
    end
end

printh("trivial cos cases")
assert_isclose(cos(0), 1.0)
assert_isclose(cos(-1), 1.0)
assert_isclose(cos(1), 1.0)
assert_isclose(cos(0.5), -1.0)
assert_isclose(cos(0.75), 0.0)
printh("other cos cases")
assert_isclose(cos(-0.5), -1.0)
assert_isclose(cos(-0.75), 0.0)
assert_isclose(cos(-0.9), 0.8089)
printh("beyond normal range cases")
assert_isclose(cos(100), 1.0)

printh("trivial atan2 cases")
assert_isclose(atan2(1, 0), 0.0)
assert_isclose(atan2(1, 1), 0.875)
assert_isclose(atan2(0, 1), 0.75)
printh("negative atan2 cases")
assert_isclose(atan2(-1, 1), 0.625)
assert_isclose(atan2(-1, 0), 0.5)
assert_isclose(atan2(-1, -1), 0.375)
assert_isclose(atan2(0, -1), 0.25)
assert_isclose(atan2(1, -1), 0.125)

printh("large unit vectors")
assert_isclose(atan2(99, 99), 0.875)

printh("special atan2 cases")
assert_isclose(atan2(0, 0), 0.25)

printh("====DONE====")
