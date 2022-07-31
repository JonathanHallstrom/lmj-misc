#pragma once

#include <numeric>
#include <cassert>
#include <limits>
#include <array>
#include <cstdint>
#include "lmj_container_helpers.hpp"

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
        return !sizeof...(Args) || (... && std::is_same_v<Args, first_type_t<Args...>>);
    }

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

        constexpr static_vector(static_vector const &) = default;

        constexpr static_vector(static_vector &&) noexcept = default;

        template<std::size_t _other_capacity>
        constexpr explicit static_vector(static_vector<T, _other_capacity> const &_other) {
            assert(_other._size <= _capacity);
            _size = _other._size;
            for (size_type i = 0; i < _size; ++i)
                _data[i] = _other._data[i];
        }

        template<std::size_t _other_capacity>
        constexpr explicit static_vector(static_vector<T, _other_capacity> &&_other) noexcept {
            assert(_other._size <= _capacity);
            _size = _other._size;
            for (size_type i = 0; i < _size; ++i)
                _data[i] = std::move(_other._data[i]);
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
            _data[_size++] = T(std::forward<Args &&...>(_args...));
            return _data[_size - 1];
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

        template<std::size_t _other_capacity>
        constexpr auto operator==(static_vector<T, _other_capacity> const &_other) const {
            if (_size != _other._size)
                return false;
            for (size_type i = 0; i < _size; ++i)
                if (_data[i] != _other._data[i])
                    return false;
            return true;
        }

        template<class G>
        constexpr auto operator!=(G const &other) {
            return !(*this == other);
        }
    };

    template<class... Args>
    constexpr auto make_static_vector(Args &&... args) {
        std::array<first_type_t<Args...>, sizeof...(Args)> arr{std::forward<Args>(args)...};
        return static_vector<first_type_t<Args...>, sizeof...(Args)>{arr.begin(), arr.end()};
    }

    // testing
    static_assert(std::is_same_v<static_vector<int, 3>, decltype(make_static_vector(1, 2, 3))>);
    static_assert([] {
        static_vector<int, 1> v;
        return v.push_back(1) == 1;
    }());
    static_assert([] {
        static_vector<int, 1> a;
        a.push_back(1);
        auto b = make_static_vector(1);
        return a == b;
    }());
    static_assert([] {
        static_vector<int, 2> a;
        a.push_back(1);
        return a == make_static_vector(1);
    }());
}

