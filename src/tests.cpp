#include "lmj_io/lmj_io.hpp"
#include "lmj_math/lmj_math.hpp"
#include "lmj_containers/lmj_containers.hpp"
#include "lmj_utils/lmj_utils.hpp"

#include <limits>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <map>

namespace lmj {
    void apply_to_all_recursively(auto &x, auto &&f) {
        if constexpr (iterable<decltype(x)>) {
            for (auto &i: x) apply_to_all_recursively(i, f);
        } else {
            f(x);
        }
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    std::cerr.tie(nullptr);
    {
        static_assert(lmj::abs(lmj::log(2) + lmj::log(3) - lmj::log(2 * 3)) < 1e-5);
    }
    {
        constexpr auto result_map = []() {
            lmj::static_hash_table<int, int, 128> map;
            for (int i = 0; i < 50; ++i)
                map[i] = i;
            return map;
        }();
        static_assert(result_map.at(2) == 2);
    }
    {
        lmj::hash_table<int, int> map;
        map[2] = 2;
        const auto map2 = map;
        assert(map2.at(2) == 2);
    }
    {
        static_assert(sizeof(lmj::helper_funcs::needed_uint<std::numeric_limits<std::uint8_t>::min()>()) == 1);
        static_assert(sizeof(lmj::helper_funcs::needed_uint<std::numeric_limits<std::uint8_t>::max()>()) == 1);
        static_assert(sizeof(lmj::helper_funcs::needed_uint<std::numeric_limits<std::uint16_t>::max()>()) == 2);
        static_assert(sizeof(lmj::helper_funcs::needed_uint<std::numeric_limits<std::uint32_t>::max()>()) == 4);
        static_assert(sizeof(lmj::helper_funcs::needed_uint<std::numeric_limits<std::uint64_t>::max()>()) == 8);
    }
    {
        constexpr auto f = lmj::lagrange::get_function(0, 0, 0.5, 0.25, 1, 1); // y = x^2
        static_assert(f(0) == 0);
        static_assert(f(1) == 1);
        static_assert(f(2) == 4);
        static_assert(f(3) == 9);
        static_assert(lmj::lagrange::get_function(0, 0, 2, 3, 5, 20)(3) == 7);
    }
    {
        static_assert(lmj::hypot(3, 4) == 5);
        static_assert(lmj::sqrt(25) == 5);
        static_assert(lmj::sqrt(9) == 3);
        static_assert(lmj::sum_squares(-9, 3) == 90);
        static_assert(lmj::ipow(0.5, 4) == 0.0625);
        for (int i = 0; i < 10000; ++i)
            assert(lmj::abs(std::exp((long double) i) - lmj::exp(i)) / std::exp((long double) i) < 1e-15);
    }
    {
        std::cerr << std::setprecision(100000);
        std::array<std::array<int, 20>, 20> ar{};
        int idx = 0;
        lmj::apply_to_all_recursively(ar, [&](auto &x) { x = 1 + idx++ % 9; });
        lmj::debug(ar);
        lmj::debug("hej hopp");
        lmj::debug(1, 2, 3);
        lmj::debug(lmj::abs(lmj::exp(1000.5l) - std::exp(1000.5l)));
        lmj::debug(lmj::exp(10));
    }
    {
        constexpr int NUM_TESTS = 1e3;
        std::unordered_map<int, int> cmp;
        lmj::hash_table<int, int> stud;
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key, value;
            key = i / 2;
            value = i;
            if (i > NUM_TESTS / 32)
                value = -i;
            cmp[key] = value;
            stud[key] = value;
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            cmp.erase(key);
            stud.erase(key);
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            int val = lmj::rand<int>();
            cmp[key] = val;
            stud[key] = val;
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            cmp.erase(key);
            stud.erase(key);
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            int val = lmj::rand<int>();
            cmp[key] = val;
            stud[key] = val;
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            for (int j = 0; j < 100; ++j) {
                int val = lmj::rand<int>();
                cmp[key] = val;
                stud[key] = val;
            }
        }

        for (auto &[key, val]: cmp)
            assert(stud[key] == val);
    }

}