#pragma once

#include <string_view>
#include <thread>
#include "../containers/containers.hpp"

namespace lmj {
template<std::size_t n, class... T>
constexpr decltype(auto) get_nth(T &&... xs) {
    return std::get<n>(std::tuple<T &&...>{xs...});
}

template<class... T>
constexpr decltype(auto) get_first(T &&... xs) {
    return get_nth<0>(xs...);
}

template<class... T>
constexpr decltype(auto) get_last(T &&... xs) {
    return get_nth<sizeof...(T) - 1>(xs...);
}

constexpr std::size_t compute_hash(char const *s, std::size_t size) { // FNV hash
    constexpr auto IS_64BIT = sizeof(std::size_t) == 8;
    std::size_t result = IS_64BIT ? 14695981039346656037ULL : 2166136261U;
    for (std::size_t i = 0; i < size; ++i) {
        result ^= s[i];
        result *= IS_64BIT ? 1099511628211ULL : 16777619U;
    }
    return result;
}

constexpr std::size_t seed_from_str(std::string_view v) {
    return compute_hash(&*v.begin(), v.size());
}

class constexpr_rand_generator { // based on xorshift random number generator by George Marsaglia
    std::uint64_t x, y, z;

public:
    constexpr explicit constexpr_rand_generator(std::size_t seed = seed_from_str(__TIME__)) : x{}, y{}, z{} {
        set_seed(seed);
    }

    constexpr void set_seed(std::uint64_t seed) {
        x = 230849599040350201 ^ static_cast<std::uint64_t>(seed);
        y = 965937400815267857 ^ static_cast<std::uint64_t>(seed);
        z = 895234450760720011 ^ static_cast<std::uint64_t>(seed);
        for (int i = 0; i < 128; ++i)
            gen<int>(); // discard first 128 values
    }

    constexpr std::uint64_t compute() {
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        const std::size_t t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;
        return z;
    }

    template<class T = std::size_t, typename = typename std::enable_if_t<std::is_integral_v<T>, void>>
    constexpr T gen() {
        return static_cast<T>(compute());
    }

    template<class T = std::size_t, typename = typename std::enable_if_t<std::is_integral_v<T>, void>>
    constexpr T randint(T lo, T hi) {
        if (lo == 0 && hi == 0xFFFFFFFFFFFFFFFF)
            return compute();
        const std::uint64_t range = hi - lo + 1;
        const std::uint64_t acceptable_range_values = 0xFFFFFFFFFFFFFFFF / range * range;
        std::uint64_t res = compute();
        if (res >= acceptable_range_values)
            res = compute();
        if (res >= acceptable_range_values)
            res = compute();
        return static_cast<T>(lo + res % range);
    }


    template<class T = std::size_t, typename = typename std::enable_if_t<std::is_integral_v<T>, void>>
    constexpr auto operator()() {
        return gen<T>();
    }
};

template<class T>
constexpr int sign(T const &x) {
    return (x > 0) - (x < 0);
}

namespace detail {
inline thread_local constexpr_rand_generator gen{seed_from_str(__TIME__) ^ [] {
    const auto id = std::this_thread::get_id();
    return *reinterpret_cast<std::uint64_t const *>(&id);
}()};
}

inline void srand(unsigned long long seed) {
    detail::gen.set_seed(seed);
}

template<class T = unsigned long long>
inline auto rand() -> std::enable_if_t<std::is_integral_v<T>, T> {
    return detail::gen.gen<T>();
}

template<class T = unsigned long long>
inline auto randint(T lo, T hi) -> std::enable_if_t<std::is_integral_v<T>, T> {
    return detail::gen.randint<T>(lo, hi);
}

constexpr auto random_shuffle(auto &random_access_container) {
    const auto n = random_access_container.size();
    using T = std::remove_cvref_t<decltype(n)>;
    for (T i = 0; i < n - 1; ++i) {
        std::swap(random_access_container[i],
                  random_access_container[i + (rand<T>() % (n - i))]);
    }
}

template<class T, class G>
constexpr auto min(T const &a, G const &b) noexcept -> decltype(a + b) {
    return a < b ? a : b;
}

template<class T, class G>
constexpr auto max(T const &a, G const &b) noexcept -> decltype(a + b) {
    return a > b ? a : b;
}

template<class T, class... G>
constexpr auto min(T const &a, G const &... b) noexcept {
    return min(a, min(b...));
}

template<class T, class... G>
constexpr auto max(T const &a, G const &... b) noexcept {
    return max(a, max(b...));
}

template<class Iterable>
constexpr auto min(Iterable const &iterable) ->
std::remove_cvref_t<decltype(std::begin(iterable), std::end(iterable), std::declval<std::iter_value_t<Iterable>>())> {
    using T = std::iter_value_t<Iterable>;
    auto iter = std::begin(iterable), end = std::end(iterable);
    T result = *iter;
    while (++iter < end)
        result = min(result, *iter);
    return result;
}

template<class Iterable>
constexpr auto max(Iterable const &iterable) ->
std::remove_cvref_t<decltype(std::begin(iterable), std::end(iterable), std::declval<std::iter_value_t<Iterable>>())> {
    using T = std::iter_value_t<Iterable>;
    auto iter = std::begin(iterable), end = std::end(iterable);
    T result = *iter;
    while (++iter < end)
        result = max(result, *iter);
    return result;
}

template<class T, std::size_t sz, class Less = std::less<>>
constexpr T min(T const (&arr)[sz]) {
    static_assert(sz >= 1);
    T result = arr[0];
    for (std::size_t i = 1; i < sz; ++i) {
        if (Less{}(arr[i], result)) {
            result = arr[i];
        }
    }
    return result;
}

template<class T, std::size_t sz, class Greater = std::greater<>>
constexpr T max(T const (&arr)[sz]) {
    static_assert(sz >= 1);
    T result = arr[0];
    for (std::size_t i = 1; i < sz; ++i) {
        if (Greater{}(arr[i], result)) {
            result = arr[i];
        }
    }
    return result;
}

// tests (very incomplete)

static_assert(min(1, 2) == 1);
static_assert(min(1, 2, 3) == 1);

static_assert(max(1, 2) == 2);
static_assert(max(1, 2, 3) == 3);

static_assert(sign(0) == 0);
static_assert(sign(1) == 1);
static_assert(sign(-1) == -1);

static_assert(min(std::array{1, 2, 3}) == 1);
static_assert(max(std::array{1, 2, 3}) == 3);
}
