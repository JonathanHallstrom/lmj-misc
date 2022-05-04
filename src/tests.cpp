#include "lmj_include_all.hpp"

#include <limits>
#include <cmath>

int main() {
    {
        static_assert(lmj::abs(lmj::log(2) + lmj::log(3) - lmj::log(2 * 3)) < 1e-5);
        lmj::print("Test 1 passed!");
    }
    {
        constexpr auto result_map = []() {
            lmj::static_hash_table<int, int, 128> map;
            for (int i = 0; i < 50; ++i)
                map[i] = i;
            return map;
        }();
        constexpr auto sum = [&]() {
            auto res = 0;
            for (int i = 0; i < 50; ++i)
                res += result_map.at(i);
            return res;
        }();
        static_assert(sum == 50 * 49 / 2);
        lmj::print("Test 2 passed!");
    }
    {
        lmj::hash_table<int, int> map;
        for (int i = 0; i < 1024; ++i)
            map[i] = i;
        for (int i = 0; i < 1024; i += 2)
            map.erase(i);
        int sum = 0;
        for (int i = 0; i < 1024; ++i)
            sum += map[i];
        assert(sum == 512 * 512); // sum of odd numbers up to and including 1023 = ((1023 - 1) / 2) ^ 2
        lmj::print("Test 3 passed!");
    }
    {
        static_assert(sizeof(lmj::needed_uint<std::numeric_limits<std::uint8_t>::min()>()) == 1);
        static_assert(sizeof(lmj::needed_uint<std::numeric_limits<std::uint8_t>::max()>()) == 1);
        static_assert(sizeof(lmj::needed_uint<std::numeric_limits<std::uint16_t>::max()>()) == 2);
        static_assert(sizeof(lmj::needed_uint<std::numeric_limits<std::uint32_t>::max()>()) == 4);
        static_assert(sizeof(lmj::needed_uint<std::numeric_limits<std::uint64_t>::max()>()) == 8);
        constexpr auto f = lmj::lagrange::get_function(0, 0, 0.5, 0.25, 1, 1); // y = x^2
        static_assert(f(0) == 0);
        static_assert(f(1) == 1);
        static_assert(f(2) == 4);
        static_assert(f(3) == 9);
        static_assert(lmj::lagrange::get_function(0, 0, 2, 3, 5, 20)(3) == 7);
        lmj::print("Test 4 passed!");
    }
    {
        static_assert(lmj::hypot(3, 4) == 5);
        static_assert(lmj::sqrt(25) == 5);
        static_assert(lmj::sqrt(9) == 3);
        static_assert(lmj::sum_squares(-9, 3) == 90);
        static_assert(lmj::ipow(0.5, 4) == 0.0625);
        static_assert(lmj::abs(lmj::exp(10) - 22026.465794806716516) < 1e-5);
        assert(lmj::abs(lmj::exp(1000.5l) - std::exp(1000.5l)) < 1e-5);
        for (int i = 0; i < 10000; ++i)
            assert(lmj::abs(std::exp(static_cast<long double>(i)) - lmj::exp(i)) /
                   std::exp(static_cast<long double>(i)) < 1e-15);
        lmj::print("Test 5 passed!");
    }
    {
        constexpr int NUM_TESTS = 1e3;
        std::unordered_map<int, int> map;
        lmj::hash_table<int, int> check;
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key, value;
            key = i / 2;
            value = i;
            if (i > NUM_TESTS / 32)
                value = -i;
            map[key] = value;
            check[key] = value;
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            map.erase(key);
            check.erase(key);
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            int val = lmj::rand<int>();
            map[key] = val;
            check[key] = val;
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            map.erase(key);
            check.erase(key);
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            int val = lmj::rand<int>();
            map[key] = val;
            check[key] = val;
        }
        for (int i = 0; i < NUM_TESTS; ++i) {
            int key = lmj::rand<int>();
            for (int j = 0; j < 100; ++j) {
                int val = lmj::rand<int>();
                map[key] = val;
                check[key] = val;
            }
        }
        for (auto &[key, val]: map)
            assert(check[key] == val);
        lmj::print("Test 6 passed!");
    }
    {
        lmj::static_hash_table<int, int, 2> t;
        t[2] = 0;
        t[4] = 0;
        t.erase(2);
        t[1] = 1;
        assert(t.at(1) == 1);
    }
    {
        constexpr auto table_1 = []() {
            lmj::static_hash_table<int, int, 128> t;
            for (int i = 0; i < 100; ++i)
                t[i] = i;
            return t;
        }();
        constexpr auto table_2 = []() {
            lmj::static_hash_table<int, int, 128> t;
            lmj::static_vector<int, 128> temp_vec;
            size_t state = 1371463783;
            for (int i = 0; i < 100; ++i) {
                state = state * 2901110977 + 1703049143;
                temp_vec.push_back(state);
                t[(int) state] = 0xBADF00D;
            }
            for (int i = 0; i < 100; ++i) {
                t.erase(temp_vec[i]);
                t[i] = i;
            }
            return t;
        }();
        static_assert(table_1 == table_2);
        lmj::print("Test 7 passed!");
    }
    {
        static_assert(lmj::abs(lmj::integrate([](auto x) { return x * x; }, 0, 3, 1e5) - 9) < 1e-3);
        lmj::print("Test 8 passed!");
    }
    {
        lmj::hash_table<int, int> map;
        std::unordered_map<int, int> check;
        std::vector<int> vals;

        assert(map.size() == check.size());

        constexpr int n = 1 << 20;

        for (int i = 0; i < n; ++i) vals.push_back(lmj::rand<int>());

        for (int i = 0; i < n; i += 2) {
            map[vals[i]] = 0xBADF00D;
            check[vals[i]] = 0xBADF00D;
        }

        assert(map.size() == check.size());

        for (auto &[key, val]: check)
            assert(map.at(key) == val);

        for (int i = 0; i < n; i += 2) {
            map.erase(vals[i]);
            check.erase(vals[i]);
        }

        assert(map.size() == check.size());

        lmj::print("Test 9 passed!");
    }
    lmj::print("All tests passed!");
}
