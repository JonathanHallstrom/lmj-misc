#include "lmj_include_all.hpp"

#include <cmath>

int main() {
    {
        // ensure basic functionality of lmj::hash_table
        lmj::hash_table<int, int> map;
        for (int i = 0; i < 1024; ++i)
            map[i] = i;
        for (int i = 0; i < 1024; i += 2)
            map.erase(i);
        int sum = 0;
        for (int i = 0; i < 1024; ++i)
            sum += map[i];
        assert(sum == 512 * 512); // sum of odd numbers up to and including 1023 = ((1023 - 1) / 2) ^ 2
        lmj::print("Test 1 passed!");
    }
    {
        assert(lmj::abs(lmj::exp(1000.5l) - std::exp(1000.5l)) < 1e-5);
        for (int i = 0; i < 10000; ++i)
            assert(lmj::abs(std::exp(static_cast<long double>(i)) - lmj::exp(i)) /
                   std::exp(static_cast<long double>(i)) < 1e-15);
        lmj::print("Test 2 passed!");
    }
    {
        // miscellaneous test of lmj::hash_table
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
        lmj::print("Test 3 passed!");
    }
    {
        // test removing elements from lmj::hash_table using std::unordered_map to ensure correctness
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

        for (auto &[key, val]: check)
            assert(map.at(key) == val);

        assert(map.size() == check.size());

        lmj::print("Test 4 passed!");
    }
    {
        // test copy constructor
        lmj::hash_table<int, int> m;
        for (int i = 0; i < 128; ++i)
            m[i] = i;
        for (auto &[key, value]: m) {
            assert(key == value);
        }
        lmj::hash_table<int, int> const m2 = m;
        for (auto &[key, value]: m2)
            assert(key == value);
        lmj::print("Test 5 passed!");
    }
    {
        // test lmj::hash_table with a custom hash function
        auto hash = [](int x) { return x * x; };
        lmj::hash_table<int, int, decltype(hash)> m(hash);
        for (int i = 0; i < 1024; ++i)
            m[i] = i;
        for (auto &[key, value]: m) {
            assert(key == value);
        }
        lmj::print("Test 6 passed!");
    }
    lmj::print("All tests passed!");
}
