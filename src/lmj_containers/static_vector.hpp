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

        /**
         * @param _n number of elements
         * @param _value value elements are set to
         */
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

        /**
         * @param element
         * @return reference to element
         */
        template<class G>
        constexpr auto &push_back(G &&_elem) {
            return emplace_back(std::forward<G>(_elem));
        }

        /**
         *
         * @param _args arguments for constructing element
         * @return reference to newly constructed element
         */
        template<class...Args>
        constexpr auto &emplace_back(Args &&... _args) {
            assert(_size < _capacity && "out of space in static_vector");
            return _data[_size++] = T(std::forward<Args &&...>(_args...));
        }

        /**
         * @param _idx index
         * @return element at index
         */
        [[nodiscard]] constexpr auto &operator[](size_type _idx) {
            return _data[_idx];
        }

        /**
         * @param index
         * @return element at index
         */
        constexpr auto const &operator[](size_type _idx) const {
            assert(_idx < _size && "no element to return");
            return _data[_idx];
        }

        /**
         * @return first element
         */
        [[nodiscard]] constexpr auto &front() {
            assert(_size && "no element to return");
            return _data[0];
        }

        /**
         * @return first element
         */
        [[nodiscard]] constexpr auto const &front() const {
            assert(_size && "no element to return");
            return _data[0];
        }

        /**
         * @return last element
         */
        [[nodiscard]] constexpr auto &back() {
            assert(_size && "no element to return");
            return _data[_size - 1];
        }

        /**
         * @return last element
         */
        [[nodiscard]] constexpr auto const &back() const {
            assert(_size && "no element to return");
            return _data[_size - 1];
        }

        /**
         * @brief removes last element
         */
        constexpr void pop_back() {
            assert(_size && "no element to pop");
            _data[--_size].~T();
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
         * @return bool indicating whether vector is empty
         */
        [[nodiscard]] constexpr bool empty() {
            return !_size;
        }

        /**
         * @return pointer to beginning of elements
         */
        [[nodiscard]] constexpr auto begin() const {
            return _data;
        }

        /**
         * @return pointer to one past the end of elements
         */
        [[nodiscard]] constexpr auto end() const {
            return _data + _size;
        }

        auto rbegin() = delete;

        auto rend() = delete;
    };
}
