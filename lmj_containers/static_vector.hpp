#pragma once

#include "../lmj_utils/lmj_utils.hpp"
#include <numeric>
#include <limits>

namespace lmj {
    template<class T, std::size_t _capacity>
    struct static_vector {
        using size_type = decltype(helper_funcs::needed_uint<_capacity>());

        T _data[_capacity]{};
        size_type _size{};

        // default constructor
        constexpr static_vector() = default;

        // size constructor
        // for example:
        // static_vector<int, 10> foo(5);
        // creates a static_vector with capacity 10, size 5 and all elements set to 0
        constexpr explicit static_vector(size_type _n, T const &_value = T{}) {
            _size = _n;
            for (T &_elem: *this) {  // initialize all elements of vector
                _elem = _value;
            }
        }

        [[nodiscard]] constexpr std::size_t size() const {
            return _size;
        }

        [[nodiscard]] constexpr std::size_t capacity() const {
            return _capacity;
        }

        // forwards to emplace_back
        template<class G>
        constexpr auto &push_back(G &&elem) {
            return emplace_back(std::forward<G>(elem));
        }

        // constructs element at index _size, if _size = _capacity you get undefined behaviour
        template<class...Args>
        constexpr auto &emplace_back(Args &&... args) {
            return _data[_size++] = T(std::forward<Args &&...>(args...));
        }

        // subscript operator
        // for example
        // static_vector<int, 10> foo(5);
        // int bar = foo[2]; <- subscript operator
        [[nodiscard]] constexpr auto &operator[](size_type idx) {
            return _data[idx];
        }

        // non-modifying subscript operator
        constexpr auto const &operator[](size_type idx) const {
            return _data[idx];
        }

        // get first element
        [[nodiscard]] constexpr auto &front() {
            return _data[0];
        }

        // get first element
        [[nodiscard]] constexpr auto const &front() const {
            return _data[0];
        }

        // get last element
        [[nodiscard]] constexpr auto &back() {
            return _data[_size - 1];
        }

        // get last element
        [[nodiscard]] constexpr auto const &back() const {
            return _data[_size - 1];
        }

        // return last element and decrease size by 1
        // DOESN'T ACTUALLY DESTROY ANY ELEMENTS
        constexpr auto pop_back() {
            return _data[--_size];
        }

        // set size to 0
        // DOESN'T ACTUALLY DESTROY ANY ELEMENTS
        constexpr void clear() {
            _size = 0;
        }

        [[nodiscard]] constexpr bool empty() {
            return !_size;
        }

        // pointer to beginning of data_format
        [[nodiscard]] constexpr auto begin() const {
            return _data;
        }

        // pointer to one past the end for iteration
        [[nodiscard]] constexpr auto end() const {
            return _data + _size;
        }

        auto rbegin() = delete;

        auto rend() = delete;
    };
}
