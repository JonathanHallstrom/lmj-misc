#pragma once

#include <chrono>
#include "../lmj_io/lmj_io.hpp"

namespace lmj {
    void debug(auto &&x);

    using namespace std::chrono;

    struct timer {
        time_point<high_resolution_clock> start_time = high_resolution_clock::now();

        bool print = true;

        timer() = default;

        explicit timer(bool p) : print{p} {}

        [[nodiscard]] auto curr_time() const {
            auto now = high_resolution_clock::now();
            auto dur = duration_cast<nanoseconds>(now - start_time);
            return double(dur.count()) / 1e9;
        }

        ~timer() {
            if (print) {
                lmj::debug(curr_time());
            }
        }
    };
}