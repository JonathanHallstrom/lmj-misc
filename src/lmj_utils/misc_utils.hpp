#pragma once

namespace lmj {
    int sign(auto &&x) {
        return (x > 0) - (x < 0);
    }

    template<class T = unsigned long long>
    auto rand() { // based on xorshift random number generator by George Marsaglia
        static unsigned long long x = 230849599040350201, y = 965937400815267857, z = 895234450760720011;
        unsigned long long t;
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;

        return (T) z;
    }
}