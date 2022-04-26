#pragma once

#include <iostream>
#include "print_impl.hpp"

namespace lmj {
    void debug(auto &&x) {
        print_impl_pretty(std::cerr, x);
        print_impl_pretty(std::cerr, '\n');
    }

    void debug(auto &&x, auto &&...pack) {
        print_impl_pretty(std::cerr, x);
        print_impl_pretty(std::cerr, ' ');
        debug(pack...);
    }
}