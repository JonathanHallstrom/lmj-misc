#pragma once

#include <chrono>
#include <iostream>

namespace lmj {
using namespace std::chrono;

struct timer {
    time_point<high_resolution_clock> start_time = high_resolution_clock::now();

    bool print = true;

    timer() = default;

    explicit timer(bool p) : print{p} {}

    [[nodiscard]] auto curr_time() const {
        auto now = high_resolution_clock::now();
        auto dur = duration_cast<nanoseconds>(now - start_time);
        return static_cast<double>(dur.count()) / 1e9;
    }

    [[nodiscard]] auto elapsed() const {
        return curr_time();
    }

    [[nodiscard]] std::string elapsed_pretty() const {
        const auto now = high_resolution_clock::now();
        const auto dur = duration_cast<nanoseconds>(now - start_time);
        const auto elapsed_nanoseconds = dur.count();
        const auto elapsed_microseconds = elapsed_nanoseconds / 1000.0;
        const auto elapsed_milliseconds = elapsed_microseconds / 1000.0;
        const auto elapsed_seconds = elapsed_milliseconds / 1000.0;
        const auto elapsed_minutes = elapsed_seconds / 60.0;
        const auto elapsed_hours = elapsed_minutes / 60.0;
        const auto elapsed_days = elapsed_hours / 24.0;
        const auto elapsed_weeks = elapsed_days / 7.0;
        auto in_range = [](auto x) {
            return 1.0 <= x && x <= 1000.0;
        };
        if (in_range(elapsed_nanoseconds))
            return std::to_string(elapsed_nanoseconds) + "ns";
        if (in_range(elapsed_microseconds))
            return std::to_string(elapsed_microseconds) + "us";
        if (in_range(elapsed_milliseconds))
            return std::to_string(elapsed_milliseconds) + "ms";
        if (in_range(elapsed_seconds))
            return std::to_string(elapsed_seconds) + "s";
        if (in_range(elapsed_minutes))
            return std::to_string(elapsed_minutes) + "m";
        if (in_range(elapsed_hours))
            return std::to_string(elapsed_hours) + "h";
        if (in_range(elapsed_days))
            return std::to_string(elapsed_days) + "d";
        return std::to_string(elapsed_weeks) + "w";
    }

    ~timer() {
        if (print) {
            std::cout.flush();
            std::cerr.flush();
            fflush(nullptr);
            std::cerr << (elapsed_pretty() + "\n");
        }
    }
};
} // namespace lmj