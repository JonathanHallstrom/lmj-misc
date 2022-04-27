#pragma once

namespace lmj {
    int sign(auto &&x) {
        return (x > 0) - (x < 0);
    }
}