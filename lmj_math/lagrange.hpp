#pragma once

#include <cstdint>
#include <utility>
#include <array>
#include "../lmj_utils/lmj_utils.hpp"
#include "../lmj_containers//lmj_containers.hpp"

namespace lmj::lagrange {
    template<std::size_t length, class...T>
    constexpr auto format_helper(static_vector<point, length> &arr, long double a, long double b, T &&...pack) {
        arr.push_back(point{a, b});
        if constexpr (sizeof...(pack)) {
            format_helper(arr, pack...);
        }
    }

    template<class... T>
    constexpr auto data_format(T &&...pack) {
        static_assert(sizeof...(T) % 2 == 0, "even number of arguments required, interpreted as pairs of (x, y)");
        constexpr auto num_pairs = sizeof...(T) / 2;
        static_vector<point, num_pairs> v;
        format_helper(v, pack...);
        std::array<point, num_pairs> res{};
        std::copy(v.begin(), v.end(), res.begin());
        return res;
    }

    template<std::size_t length>
    constexpr long double interpolate(long double x, std::array<point, length> const &points) {
        long double result = 0;
        for (std::size_t i = 0; i < length; ++i) {
            long double p = points[i].y;
            for (std::size_t j = 0; j < length; ++j) {
                if (i == j)
                    continue;
                p *= (x - points[j].x) / (points[i].x - points[j].x);
            }
            result += p;
        }
        return result;
    }

    /**
     * @brief get polynomial function which has values of (x, y) as specified in parameter points
     * @param points
     * @return
     */
    constexpr auto get_function(Numbers auto &&... points) {
        return [points...](long double x) { return interpolate(x, data_format(points...)); };
    }
}