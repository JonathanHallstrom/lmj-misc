#pragma once

namespace lmj {
    template<std::size_t n>
    constexpr auto needed_uint() {
        if constexpr (n <= std::numeric_limits<std::uint8_t>::max()) {
            return std::uint8_t{};
        } else if constexpr (n <= std::numeric_limits<std::uint16_t>::max()) {
            return std::uint16_t{};
        } else if constexpr (n <= std::numeric_limits<std::uint32_t>::max()) {
            return std::uint32_t{};
        } else {
            return std::uint64_t{};
        }
    }
}