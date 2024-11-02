#include <string_view>

// TODO: minify (or eliminate) lua header
/// @brief This header is injected at the start of every cart.
/// Whatever symbols it brings to the `local` scope will be visible to the cart.
static constexpr std::string_view app_header =
	R"(
local all = function(t)
	if t == nil or #t == 0 then
		return function() end
	end

	local i = 1
	local prev = nil

	return function()
		if t[i] == prev then
			i += 1
		end

		while t[i] == nil and i <= #t do
			i += 1
		end

		prev = t[i]

		return prev
	end
end

local inext = function(t, idx)
	if (idx == nil) idx = 1 else idx += 1
	local x = t[idx]
	if (x == nil) return nil
	return idx, x
end

local count = function(t, v)
	local n = 0
	if v == nil then
		for i = 1,#t do
			if t[i] ~= nil then
				n += 1
			end
		end
	else
		for i = 1,#t do
			if t[i] == v then
				n += 1
			end
		end
	end
	return n
end

function __panic(msg)
	printh("PANIC: " .. msg)
	print(":(", 0, 0, 7)
	print(msg)
end
)"

	/// This re-exports certain globals into the `local` scope of the cart.
    /// Calling these is faster, but there is a 200 limit on locals (including
    /// the cart's own), so it shouldn't be used a ton. That limit only affects
    /// the number of locals that are used, AFAICT.
    ///
    /// This should preferably only re-export standard functions that are used
    /// in hot loops.
    ///
    /// When re-exported in the `local` scope, any use gets added to the upvalue
    /// table of a function. Usually, this is a net performance benefit, because
    /// upvalue accesses can be done directly rather than through an environment
    /// lookup by string key (even if this usecase was optimized for in y8).
    ///
    /// However, it also results in longer upvalue construction time
    /// per-function... For how carts normally behave, this is probably fine.

	R"(local color, pset, pget, sset, sget, fget, line, circfill, rectfill, spr, sspr, pal, palt, fillp, clip, mset, mget, map, peek, peek2, peek4, poke, poke2, poke4, memcpy, memset, abs, flr, mid, min, max, sin, cos, sqrt, shl, shr, band, bor, rnd, t, time, add, foreach, split, unpack, ord =
color, pset, pget, sset, sget, fget, line, circfill, rectfill, spr, sspr, pal, palt, fillp, clip, mset, mget, map, peek, peek2, peek4, poke, poke2, poke4, memcpy, memset, abs, flr, mid, min, max, sin, cos, sqrt, shl, shr, band, bor, rnd, t, time, add, foreach, split, unpack, ord
)"

	R"(_gc(); printh(stat(0) .. "KB at boot"))";