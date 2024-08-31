#pragma once

#include <algorithm>

namespace util {

struct Point {
	constexpr Point() = default;

	constexpr Point(int x, int y) : x(x), y(y) {}

	constexpr Point(const Point &) = default;
	constexpr Point &operator=(const Point &) = default;

	[[nodiscard]] constexpr Point max(Point other) const {
		return Point(std::max(x, other.x), std::max(y, other.y));
	}

	[[nodiscard]] constexpr Point min(Point other) const {
		return Point(std::min(x, other.x), std::min(y, other.y));
	}

	[[nodiscard]] constexpr Point with_offset(int offset_x,
	                                          int offset_y) const {
		return {x + offset_x, y + offset_y};
	}

	[[nodiscard]] constexpr Point yx() const { return {y, x}; }

	int x = 0, y = 0;
};

} // namespace util