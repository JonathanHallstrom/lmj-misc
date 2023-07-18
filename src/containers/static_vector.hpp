#pragma once

#include "container_helpers.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <numeric>
#include <type_traits>
#include <vector>

namespace lmj {
template<class... Args>
struct first_type;

template<class T, class... Args>
struct first_type<T, Args...> {
    using type = T;
};

template<class... Args>
using first_type_t = typename first_type<Args...>::type;

template<class... Args>
constexpr auto all_same_types() {
    return !sizeof...(Args) || (std::is_same_v<Args, first_type_t<Args...>> && ...);
}

template<class T, std::size_t _capacity>
class static_vector {
private:
public:
    using value_type = T;
    using reference = value_type &;
    using const_reference = value_type const &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using size_type = std::size_t;
    using difference_type = std::make_signed_t<size_type>;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    value_type m_data[_capacity]{};
    size_type m_size{};

    constexpr static_vector() = default;

    constexpr explicit static_vector(size_type n) {
        assert(n <= _capacity);
        m_size = n;
    }

    constexpr explicit static_vector(size_type n, T const &value) {
        m_size = n;
        for (size_type i = 0; i < m_size; ++i)
            m_data[i] = value;
    }

    template<class Iter>
    constexpr explicit static_vector(Iter begin, Iter end) {
        static_assert(decltype(++begin, begin != end, *begin, 0)() == 0);
        while (begin != end) {
            emplace_back(*begin);
            ++begin;
        }
    }

    constexpr static_vector(static_vector const &) = default;

    constexpr static_vector(static_vector &&) noexcept = default;

    constexpr static_vector &operator=(static_vector const &) = default;

    constexpr static_vector &operator=(static_vector &&) noexcept = default;

    template<std::size_t other_capacity>
    constexpr auto &operator=(static_vector<T, other_capacity> const &other) {
        assert(other.m_size <= _capacity);
        m_size = other.m_size;
        for (size_type i = 0; i < m_size; ++i)
            m_data[i] = other.m_data[i];
        return *this;
    }

    template<std::size_t other_capacity>
    constexpr auto &operator=(static_vector<T, other_capacity> &&other) {
        assert(other.m_size <= _capacity);
        m_size = other.m_size;
        for (size_type i = 0; i < m_size; ++i)
            m_data[i] = std::move(other.m_data[i]);
        return *this;
    }

    template<std::size_t other_capacity>
    constexpr explicit static_vector(static_vector<T, other_capacity> const &other) {
        *this = other;
    }

    template<std::size_t other_capacity>
    constexpr explicit static_vector(static_vector<T, other_capacity> &&other) noexcept {
        *this = std::move(other);
    }

    constexpr static_vector(std::initializer_list<T> il) {
        assert(il.size() <= _capacity);
        m_size = il.size();
        for (size_type i = 0; i < m_size; ++i)
            m_data[i] = il.begin()[i];
    }

    [[nodiscard]] constexpr size_type size() const {
        return m_size;
    }

    [[nodiscard]] constexpr size_type capacity() const {
        return _capacity;
    }

    [[nodiscard]] constexpr size_type max_size() const {
        return _capacity;
    }

    template<class G>
    constexpr auto &push_back(G &&elem) {
        return emplace_back(std::forward<G>(elem));
    }

    template<class... Args>
    constexpr auto &emplace_back(Args &&...args) {
        assert(m_size < _capacity && "out of space in static_vector");
        m_data[m_size++] = T(std::forward<Args>(args)...);
        return m_data[m_size - 1];
    }

    [[nodiscard]] constexpr reference operator[](size_type idx) {
        assert(idx < m_size && "no element to return");
        return m_data[idx];
    }

    [[nodiscard]] constexpr const_reference operator[](size_type idx) const {
        assert(idx < m_size && "no element to return");
        return m_data[idx];
    }

    [[nodiscard]] constexpr reference front() {
        assert(m_size && "no element to return");
        return m_data[0];
    }

    [[nodiscard]] constexpr const_reference front() const {
        assert(m_size && "no element to return");
        return m_data[0];
    }

    [[nodiscard]] constexpr reference back() {
        assert(m_size && "no element to return");
        return m_data[m_size - 1];
    }

    [[nodiscard]] constexpr const_reference back() const {
        assert(m_size && "no element to return");
        return m_data[m_size - 1];
    }

    constexpr T pop_back() {
        assert(m_size && "no element to pop");
        auto res = std::move(m_data[m_size - 1]);
        m_data[--m_size].~T();
        return res;
    }

    constexpr void erase(const_iterator iter) {
        erase(iter, iter + 1);
    }

    constexpr void erase(const_iterator first, const_iterator last) {
        assert(first >= begin() && first <= end());
        assert(last >= begin() && (last <= end() || last == end()));

        if (first == last) return;

        iterator i1 = begin() + (first - begin()), i2 = begin() + (last - begin());
        if (i1 != first || i2 != last)
#if __has_builtin(__builtin_unreachable)
            __builtin_unreachable()
#endif
                    ;

        while (i2 != end())
            *i1++ = *i2++;
        m_size -= last - first;
    }

    constexpr void clear() {
        for (size_type i = 0; i < m_size; ++i)
            m_data[i].~T();
        m_size = 0;
    }

    [[nodiscard]] constexpr bool empty() const {
        return m_size == 0;
    }

    [[nodiscard]] constexpr const_iterator begin() const {
        return const_iterator{m_data};
    }

    [[nodiscard]] constexpr const_iterator end() const {
        return begin() + m_size;
    }

    [[nodiscard]] constexpr iterator begin() {
        return iterator{m_data};
    }

    [[nodiscard]] constexpr iterator end() {
        return begin() + m_size;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const {
        return const_iterator{m_data};
    }

    [[nodiscard]] constexpr const_iterator cend() const {
        return begin() + m_size;
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() const {
        return std::reverse_iterator{end()};
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const {
        return std::reverse_iterator{begin()};
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() {
        return std::reverse_iterator{end()};
    }

    [[nodiscard]] constexpr reverse_iterator rend() {
        return std::reverse_iterator{begin()};
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const {
        return std::reverse_iterator{cend()};
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const {
        return std::reverse_iterator{cbegin()};
    }

    template<std::size_t other_capacity>
    [[nodiscard]] constexpr bool operator==(static_vector<T, other_capacity> const &other) const {
        if (m_size != other.m_size)
            return false;
        for (size_type i = 0; i < m_size; ++i)
            if (m_data[i] != other.m_data[i])
                return false;
        return true;
    }

    template<std::size_t other_capacity>
    [[nodiscard]] constexpr bool operator!=(static_vector<T, other_capacity> const &other) const {
        if (m_size != other.m_size)
            return true;
        for (size_type i = 0; i < m_size; ++i)
            if (m_data[i] != other.m_data[i])
                return false;
        return true;
    }

    [[nodiscard]] std::vector<T> to_std_vector() const {
        return std::vector(begin(), end());
    }
};


template<class... Args>
constexpr auto make_static_vector(Args &&...args) {
    static_assert(all_same_types<std::remove_cvref_t<Args>...>());
    return static_vector<std::remove_cvref_t<first_type_t<Args...>>, sizeof...(Args)>{std::forward<Args>(args)...};
}

// testing
static_assert(std::is_same_v<static_vector<int, 3>, decltype(make_static_vector(1, 2, 3))>);
static_assert([] {
    static_vector<int, 1> v;
    v.push_back(1);
    return v[0] == 1;
}());
static_assert([] {
    return static_vector<int, 2>{1} == static_vector<int, 1>{1};
}());
static_assert([] { // make sure reverse iterators work
    const static_vector<int, 3> v = {1, 2, 3};

    std::array<int, 3> arr1{}, arr2{};
    std::copy(v.begin(), v.end(), arr1.rbegin());
    std::copy(v.rbegin(), v.rend(), arr2.begin());
    return arr1 == arr2;
}());
static_assert([] {
    static_vector<int, 3> v = {1, 2, 3};
    v.erase(v.begin(), v.begin() + 2);
    return v == make_static_vector(3);
}());
static_assert([] {
    static_vector<int, 3> v1 = {1, 2, 3};
    static_vector<int, 4> v2{};
    v1 = v2;
    return true;
}());
} // namespace lmj
