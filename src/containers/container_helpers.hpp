#pragma once

#include <limits>

namespace lmj::detail {
template<std::uint64_t n>
using required_uint_t =
        typename std::conditional<
                (n <= std::numeric_limits<std::uint8_t>::max()), std::uint8_t,
                typename std::conditional<
                        (n <= std::numeric_limits<std::uint16_t>::max()), std::uint16_t,
                        typename std::conditional<
                                (n <= std::numeric_limits<std::uint32_t>::max()),
                                std::uint32_t, std::uint64_t>::type>::type>::type;
} // namespace lmj::detail