#pragma once

#include <cstdio>
#include <cstdint>
#include "../utils/utils.hpp"
#include "concepts"

namespace lmj::fast_print {

inline auto print_impl(_iobuf *fptr, floating_point auto x) {
    std::fprintf(fptr, "%Lf", static_cast<long double>(x));
}

inline auto print_impl(_iobuf *fptr, signed_integral auto x) {
    if constexpr (std::is_same_v<decltype(x), signed char> || std::is_same_v<decltype(x), char>) {
        std::fputc(x, fptr);
    } else {
        std::fprintf(fptr, "%lld", static_cast<std::int64_t>(x));
    }
}

inline auto print_impl(_iobuf *fptr, unsigned_integral auto x) {
    if constexpr (std::is_same_v<decltype(x), unsigned char> || std::is_same_v<decltype(x), char>) {
        std::fputc(x, fptr);
    } else {
        std::fprintf(fptr, "%llu", static_cast<std::uint64_t>(x));
    }
}

inline auto print_impl(_iobuf *fptr, std::string_view x) {
    for (auto i: x)
        print_impl(fptr, i);
}

inline auto print_impl_pretty(_iobuf *fptr, floating_point auto x) {
    std::fprintf(fptr, "%Lf", static_cast<long double>(x));
}

inline auto print_impl_pretty(_iobuf *fptr, signed_integral auto x) {
    if constexpr (std::is_same_v<decltype(x), signed char> || std::is_same_v<decltype(x), char>) {
        std::fputc(x, fptr);
    } else {
        std::fprintf(fptr, "%lld", static_cast<std::int64_t>(x));
    }
}

inline auto print_impl_pretty(_iobuf *fptr, unsigned_integral auto x) {
    if constexpr (std::is_same_v<decltype(x), unsigned char> || std::is_same_v<decltype(x), char>) {
        std::fputc(x, fptr);
    } else {
        std::fprintf(fptr, "%llu", static_cast<std::uint64_t>(x));
    }
}

inline auto print_impl_pretty(_iobuf *fptr, std::string_view x) {
    for (auto i: x)
        print_impl_pretty(fptr, i);
}

template<class T, class G>
auto print_impl(_iobuf *fptr, std::pair<T, G> const &x);

template<class T>
requires lmj::iterable<T> && (!std::is_convertible_v<T, std::string> && !std::is_convertible_v<T, std::string_view>)
auto print_impl(_iobuf *fptr, T &&x) {
    bool first = true;
    for (auto &&i: x) {
        if (!first)
            print_impl(fptr, ' ');
        first = false;
        print_impl(fptr, i);
    }
}

template<class T, class G>
auto print_impl(_iobuf *fptr, std::pair<T, G> const &x) {
    print_impl(fptr, x.first);
    print_impl(fptr, ' ');
    print_impl(fptr, x.second);
}

template<class T, class G>
auto print_impl_pretty(_iobuf *fptr, std::pair<T, G> const &x);

template<class T>
requires lmj::iterable<T> && (!std::is_convertible_v<T, std::string> && !std::is_convertible_v<T, std::string_view>)
auto print_impl_pretty(_iobuf *fptr, T &&x) {
    print_impl_pretty(fptr, '{');
    bool first = true;
    for (auto &&i: x) {
        if (!first) {
            if constexpr (iterable<decltype(i)> || requires { i.second; i.first; })
                print_impl_pretty(fptr, ",\n");
            else
                print_impl_pretty(fptr, ", ");
        }
        first = false;
        print_impl_pretty(fptr, i);
    }
    print_impl_pretty(fptr, '}');
}

template<class T, class G>
auto print_impl_pretty(_iobuf *fptr, std::pair<T, G> const &x) {
    print_impl_pretty(fptr, '{');
    print_impl_pretty(fptr, x.first);
    print_impl_pretty(fptr, ',');
    print_impl_pretty(fptr, x.second);
    print_impl_pretty(fptr, '}');
}


}
