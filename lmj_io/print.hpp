#pragma once

#include <iostream>
#include "print_impl.hpp"

namespace lmj {
    void print(auto &&x) {
        print_impl(std::cout, x);
        print_impl(std::cout, '\n');
    }

    void print(auto &&x, auto &&...pack) {
        print_impl(std::cout, x);
        print_impl(std::cout, ' ');
        print(pack...);
    }
}