#pragma once

namespace util
{

struct Point
{
    constexpr Point() = default;

    constexpr Point(int x, int y) :
        x(x), y(y)
    {}

    constexpr Point(const Point&) = default;
    constexpr Point& operator=(const Point&) = default;

    [[nodiscard]] constexpr Point yx() const
    {
        return {y, x};
    }

    int x = 0, y = 0;
};

}