#pragma once

#include <numeric>
#include <cassert>
#include <limits>
#include <array>
#include "lmj_container_helpers.hpp"

namespace lmj {
    template<class T, std::size_t _capacity>
    class static_vector {
    public:
        using size_type = decltype(needed_uint<_capacity>());

        T _data[_capacity]{};
        size_type _size{};

        constexpr static_vector() = default;

        /**
         * @param _n number of elements
         */
        constexpr explicit static_vector(size_type _n) {
            assert(_n <= _capacity);
            _size = _n;
        }

        /**
         * @param _n number of elements
         * @param _value value elements are set to
         */
        constexpr explicit static_vector(size_type _n, T const &_value) {
            _size = _n;
            for (size_type i = 0; i < _size; ++i)
                _data[i] = _value;
        }

        template<class Iter>
        constexpr explicit static_vector(Iter begin, Iter end) {
            static_assert(decltype(++begin, begin != end, *begin, 0)() == 0);
            while (begin != end) {
                emplace_back(*begin);
                ++begin;
            }
        }

        constexpr static_vector(std::initializer_list<T> l) {
            for (auto &&i: l)
                emplace_back(i);
        }

        /**
         * @return size of vector
         */
        [[nodiscard]] constexpr std::size_t size() const {
            return _size;
        }

        /**
         * @return capacity of vector
         */
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
         * @brief removes all elements
         */
        constexpr void clear() {
            for (size_type i = 0; i < _size; ++i)
                _data[i].~T();
            _size = 0;
        }

        /**
         * @return vector is empty
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

        /**
         * @return pointer to beginning of elements
         */
        [[nodiscard]] constexpr auto begin() {
            return _data;
        }

        /**
         * @return pointer to one past the end of elements
         */
        [[nodiscard]] constexpr auto end() {
            return _data + _size;
        }

        auto rbegin() = delete;

        auto rend() = delete;
    };

    template<class... T>
    constexpr auto make_static_vector(T &&... args) {
        using elem_type = typename decltype(std::array{args...})::value_type;
        return static_vector<elem_type, sizeof...(T)>{args...};
    }
}
