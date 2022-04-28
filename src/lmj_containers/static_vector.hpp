#pragma once

#include <numeric>
#include <limits>
#include "container_helpers.hpp"

namespace lmj {
    template<class T, std::size_t _capacity>
    class static_vector {
    public:
        using size_type = decltype(needed_uint<_capacity>());

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
            for (T &_elem: _data) {  // initialize all elements of vector
                _elem = _value;
            }
        }

        [[nodiscard]] constexpr std::size_t size() const {
            return _size;
        }

        [[nodiscard]] constexpr std::size_t capacity() const {
            return _capacity;
        }

        // forward to emplace_back
        template<class G>
        constexpr auto &push_back(G &&elem) {
            return emplace_back(std::forward<G>(elem));
        }

        // constructs element at index _size, if _size = _capacity you get undefined behaviour
        template<class...Args>
        constexpr auto &emplace_back(Args &&... args) {
            assert(_size < _capacity && "out of space in static_vector");
            return _data[_size++] = T(std::forward<Args &&...>(args...));
        }

        /**
         * @param index
         * @return element at index
         */
        [[nodiscard]] constexpr auto &operator[](size_type idx) {
            return _data[idx];
        }

        /**
         * @param index
         * @return element at index
         */
        constexpr auto const &operator[](size_type idx) const {
            return _data[idx];
        }

        /**
         * @return first element
         */
        [[nodiscard]] constexpr auto &front() {
            return _data[0];
        }

        /**
         * @return first element
         */
        [[nodiscard]] constexpr auto const &front() const {
            return _data[0];
        }

        /**
         * @return last element
         */
        [[nodiscard]] constexpr auto &back() {
            return _data[_size - 1];
        }

        /**
         * @return last element
         */
        [[nodiscard]] constexpr auto const &back() const {
            return _data[_size - 1];
        }

        /**
         * @brief removes last element
         */
        constexpr void pop_back() {
            assert(_size && "no element to pop");
            --_size;
            _data[_size].~T();
        }

        /**
         * @brief remove all elements
         */
        constexpr void clear() {
            for (size_type i = 0; i < _size; ++i)
                _data[i].~T();
            _size = 0;
        }

        /**
         * @return whether vector is empty
         */
        [[nodiscard]] constexpr bool empty() {
            return !_size;
        }

        /**
         * @return pointer to data
         */
        [[nodiscard]] constexpr auto begin() const {
            return _data;
        }

        /**
         * @return pointer to end
         */
        [[nodiscard]] constexpr auto end() const {
            return _data + _size;
        }

        auto rbegin() = delete;

        auto rend() = delete;
    };
}
