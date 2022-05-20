#pragma once

namespace lmj {
    struct point {
        long double x{}, y{};

        constexpr point() = default;

        constexpr point(auto &&_x, auto &&_y) : x(_x), y(_y) {}
    };
}