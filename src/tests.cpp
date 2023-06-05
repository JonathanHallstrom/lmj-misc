#include "include_all.hpp"

#include <cmath>
#include <future>
#include <iomanip>
#include <set>

template<class ContainerType>
concept Container = requires(ContainerType a, const ContainerType b)
{
    requires std::regular<ContainerType>;
    requires std::swappable<ContainerType>;
    requires std::destructible<typename ContainerType::value_type>;
    requires std::same_as<typename ContainerType::reference, typename ContainerType::value_type &>;
    requires std::same_as<typename ContainerType::const_reference, const typename ContainerType::value_type &>;
    requires std::forward_iterator<typename ContainerType::iterator>;
    requires std::forward_iterator<typename ContainerType::const_iterator>;
    requires std::signed_integral<typename ContainerType::difference_type>;
    requires std::same_as<typename ContainerType::difference_type, typename std::iterator_traits<typename ContainerType::iterator>::difference_type>;
    requires std::same_as<typename ContainerType::difference_type, typename std::iterator_traits<typename ContainerType::const_iterator>::difference_type>;
    { a.begin() } -> std::same_as<typename ContainerType::iterator>;
    { a.end() } -> std::same_as<typename ContainerType::iterator>;
    { b.begin() } -> std::same_as<typename ContainerType::const_iterator>;
    { b.end() } -> std::same_as<typename ContainerType::const_iterator>;
    { a.cbegin() } -> std::same_as<typename ContainerType::const_iterator>;
    { a.cend() } -> std::same_as<typename ContainerType::const_iterator>;
    { a.size() } -> std::same_as<typename ContainerType::size_type>;
    { a.max_size() } -> std::same_as<typename ContainerType::size_type>;
    { a.empty() } -> std::same_as<bool>;
};

static_assert(Container<lmj::static_vector<int, 1>>);
static_assert(Container<lmj::static_hash_table<int, int, 1>>);
static_assert(Container<lmj::hash_table<int, int>>);

int main() {
    std::atomic<std::int64_t> idx = 1;
    std::vector<std::future<void>> test_futures;
    auto mode = std::launch::async;
    bool print_times = false;
    lmj::timer t{print_times};
    const auto thread_count = static_cast<std::int64_t>(std::thread::hardware_concurrency());
    auto register_test = [&, test_idx = 1](auto &&test) mutable {
        test_futures.push_back(std::async(mode, [&, test_idx] {
            while (idx < int(test_idx - thread_count * 5 / 4))
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            lmj::timer lambda_t{false};
            test();
            auto elapsed = lambda_t.elapsed_pretty();
            while (idx < test_idx)
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            if (print_times)
                lmj::print("Test", test_idx, "passed!", elapsed);
            else
                lmj::print("Test", test_idx, "passed!");
            ++idx;
        }));
        ++test_idx;
    };
    register_test([] {
        constexpr int n = 1 << 13;
        // ensure basic functionality of lmj::hash_table
        lmj::hash_table<int, int> map;
        for (int i = 0; i < n; ++i)
            map[i] = i;
        for (int i = 0; i < n; i += 2)
            map.erase(i);
        std::uint64_t sum = 0;
        for (int i = 0; i < n; ++i)
            sum += map[i];
        assert(sum == n * n / 4);
    });
    register_test([] {
        constexpr int N = 1 << 20;
        // miscellaneous test of lmj::hash_table ("do random bullsh*t and see if the standard version ends up the same")
        std::unordered_map<int, int> map1, map2;
        lmj::hash_table<int, int> check1, check2;
        for (int i = 0; i < N; ++i) {
            {
                const int key = lmj::randint(0, 1 << 12);
                const int val = lmj::randint(0, 1 << 12);
                switch (lmj::randint(0, 9)) {
                    case 0:
                        map1[key] = val;
                        check1[key] = val;
                        break;
                    case 1:
                        map1.erase(key);
                        check1.erase(key);
                        break;

                    case 2:
                        map2[key] = val;
                        check2[key] = val;
                        break;
                    case 3:
                        map2.erase(key);
                        check2.erase(key);
                        break;

                    case 4:
                        map1 = map2;
                        check1 = check2;
                        break;
                    case 5:
                        map2 = map1;
                        check2 = check1;
                        break;

                    case 6:
                        if (!(key & 1023)) {
                            map2 = std::move(map1);
                            check2 = std::move(check1);
                            map1 = {};
                            check1 = {};
                        }
                        break;
                    case 7:
                        if (!(key & 1023)) {
                            map1 = std::move(map2);
                            check1 = std::move(check2);
                            map2 = {};
                            check2 = {};
                        }
                        break;
                    case 8:
                        std::swap(map1, map2);
                        std::swap(check1, check2);
                        break;
                }
            }

            assert(map1.size() == check1.size());
            assert(map2.size() == check2.size());
            for (const auto &[key, val]: map1)
                assert(check1.at(key) == val);
            for (const auto &[key, val]: map2)
                assert(check2.at(key) == val);
        }
    });
    register_test([] {
        constexpr int n = 1 << 18;
        // test removing elements from lmj::hash_table using std::unordered_map to ensure correctness
        lmj::hash_table<int, int> map;
        std::unordered_map<int, int> check;
        std::vector<int> vals;

        assert(map.size() == check.size());

        vals.reserve(n);
        for (int i = 0; i < n; ++i)
            vals.push_back(lmj::rand<int>());

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

        for ([[maybe_unused]] auto &[key, val]: check)
            assert(map.at(key) == val);

        assert(map.size() == check.size());
    });
    register_test([] {
        constexpr int n = 1 << 18;
        // test copy constructor
        lmj::hash_table<int, int> m;
        for (int i = 0; i < n; ++i)
            m[i] = i;
        for (auto &[key, value]: m) {
            assert(key == value);
        }
        lmj::hash_table<int, int> const m2 = m;
        for (auto &[key, value]: m2)
            assert(key == value);
    });
    register_test([] {
        constexpr int n = 1 << 18;
        // test lmj::hash_table with a custom hash function
        auto hash = [](int x) { return x * x; };
        lmj::hash_table<int, int, decltype(hash)> m;
        for (int i = 0; i < n; ++i)
            m[i] = i;
        for (auto &[key, value]: m) {
            assert(key == value);
        }
    });
    register_test([] {
        constexpr int n = 1 << 18;
        // test lmj::hash_table find
        lmj::hash_table<int, int> m;
        for (int i = 0; i < n; ++i)
            m[i] = i;
        for (int i = 0; i < n; ++i) {
            assert(m.find(i) != m.end());
            assert(m.find(i)->first == i);
        }
        assert(m.find(1024) == m.end());
    });
    register_test([] {
        constexpr int n = 1 << 14;
        // test lmj::hash_table with different types of keys and values and hash
        auto hash = [](int x) { return x * x; };
        auto str = [](int x) { return "to make it a long string " + std::to_string(x); };
        lmj::hash_table<int, std::string> m1;
        lmj::hash_table<int, std::string, decltype(hash)> m2{hash};
        for (int i = 0; i < n; ++i)
            m1[i] = m2[i] = str(i);
        for (int i = 0; i < n; ++i) {
            assert(m1.find(i) != m1.end());
            assert(m1.find(i)->first == i);
            assert(m1.find(i)->second == str(i));
        }
        assert(m1 == m2);
        assert(m1.find(n) == m1.end());
    });
    for (auto &&i: test_futures)
        i.get();
    lmj::print("All tests passed!");
}
