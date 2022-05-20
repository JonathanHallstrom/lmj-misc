#pragma once

#include <utility>
#include <functional>
#include <cstdint>

#include <lmj_utils.hpp>
#include <lmj_container_helpers.hpp>
#include <lmj_misc_utils.hpp>

namespace lmj {
    template<class key_type, class value_type, std::size_t _capacity, class hash_type = lmj::hash<key_type>>
    class static_hash_table {
    public:
        static_assert(_capacity && "a capacity of zero is not allowed");
        using pair_type = std::pair<key_type, value_type>;
        using size_type = decltype(needed_uint<_capacity>());
        using bool_type = std::uint8_t;
        pair_type _table[_capacity]{};
        bool_type _is_set[_capacity]{};
        size_type _elem_count{};
        hash_type _hasher{};

        enum active_enum : bool_type {
            INACTIVE = 0,
            ACTIVE = 1,
            TOMBSTONE = 2
        };

        constexpr static_hash_table() = default;

        constexpr static_hash_table(static_hash_table const &other) {
            *this = other;
        }

        constexpr static_hash_table(static_hash_table &&other) noexcept {
            *this = other;
        }

        constexpr explicit static_hash_table(hash_type _hasher) : _hasher(_hasher) {}

        ~static_hash_table() = default;

        constexpr static_hash_table &operator=(static_hash_table &&other) noexcept {
            *this = other;
            return *this;
        }

        constexpr static_hash_table &operator=(static_hash_table const &other) {
            if (this != &other)
                _copy(other);
            return *this;
        }

        constexpr bool operator==(static_hash_table const &other) const {
            if (other.size() != this->size())
                return false;
            for (std::size_t i = 0; i < _capacity; ++i) {
                if (_is_set[i] == active_enum::ACTIVE) {
                    size_type _idx = other._get_index_read(_table[i].second);
                    if (other._is_set[_idx] != active_enum::ACTIVE)
                        return false;
                    if (other._table[_idx].second != _table[i].second)
                        return false;
                }
            }
            return true;
        }

        /**
         * @return reference to value associated with _key or default constructs value if it doesn't exist
         */
        constexpr value_type &operator[](key_type const &_key) {
            return get(_key);
        }

        /**
         * @return value at _key or fails
         */
        constexpr value_type const &at(key_type const &_key) const {
            size_type _idx = _get_index_read(_key);
            assert(_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _key && "key not found");
            return _table[_idx].second;
        }

        /**
         * @brief gets value at _key or creates new value at _key with default value
         * @return reference to value associated with _key
         */
        constexpr value_type &get(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            return (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _key) ?
                   _table[_idx].second : emplace(_key, value_type{});
        }

        /**
         * @return whether _key is in table
         */
        constexpr bool contains(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            return _is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _key;
        }

        /**
         * @param _key key which is removed from table
         */
        constexpr void erase(key_type const &_key) {
            remove(_key);
        }

        /**
         * @param _key key which is removed from table
         */
        constexpr void remove(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            if (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _key) {
                --_elem_count;
                _table[_idx] = pair_type{};
                _is_set[_idx] = active_enum::TOMBSTONE;
            }
        }

        /**
         * @param _key
         * @param _value
         * @return reference to _value in table
         */
        constexpr value_type &insert(key_type const &_key, value_type const &_value) {
            return emplace(_key, _value);
        }

        /**
         * @param _pack arguments for constructing element
         * @return  reference to newly constructed value
         */
        constexpr value_type &emplace(auto &&... _pack) {
            static_assert(sizeof...(_pack));
            assert(_elem_count < _capacity);
            auto _p = pair_type{_pack...};
            size_type _idx = _get_index_read(_p.first);
            if (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _p.first)
                return _table[_idx].second;
            _idx = _get_writable_index(_p.first);
            ++_elem_count;
            _is_set[_idx] = active_enum::ACTIVE;
            _table[_idx] = _p;
            return _table[_idx].second;
        }

        /**
         * @return number of elements
         */
        [[nodiscard]] constexpr size_type size() const {
            return _elem_count;
        }

        /**
         * @return capacity of table
         */
        [[nodiscard]] constexpr size_type capacity() const {
            return _capacity;
        }


        /**
         * @brief remove all elements
         */
        void clear() {
            for (size_type i = 0; i < _capacity; ++i) {
                if (_is_set[i] == active_enum::ACTIVE) {
                    _table[i].~pair_type();
                }
                _is_set[i] = active_enum::INACTIVE;
            }
            _elem_count = 0;
        }

    private:
        constexpr void _copy(static_hash_table const &other) {
            for (size_type i = 0; i < other.capacity(); ++i) {
                if (other._is_set[i] == active_enum::ACTIVE) {
                    _table[i].first = other._table[i].second;
                    _table[i].second = other._table[i].second;
                }
                _is_set[i] = other._is_set[i];
            }
            _elem_count = other._elem_count;
            _hasher = other._hasher;
        }

        [[nodiscard]] constexpr size_type _clamp_size(size_type _idx) const {
            if constexpr ((_capacity & (_capacity - 1)) == 0)
                return _idx & (_capacity - 1);
            else
                return _idx % _capacity;
        }

        [[nodiscard]] constexpr size_type _get_hash(key_type const &_key) const {
            return _clamp_size(_hasher(_key));
        }

        [[nodiscard]] constexpr size_type _new_idx(size_type const _idx) const {
            return _clamp_size(_idx + 1);
        }

        [[nodiscard]] constexpr size_type _get_index_read(key_type const &_key) const {
            size_type _idx = _get_hash(_key);
            std::size_t _iterations = 0;
            while ((_is_set[_idx] == active_enum::TOMBSTONE ||
                    (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first != _key))
                   && _iterations++ < _capacity) {
                _idx = _new_idx(_idx);
            }
            return _idx;
        }

        [[nodiscard]] constexpr size_type _get_writable_index(key_type const &_key) const {
            size_type _idx = _get_hash(_key);
            std::size_t _iterations = 0;
            while (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first != _key) {
                assert(_iterations++ < _capacity && "empty index not found");
                _idx = _new_idx(_idx);
            }
            return _idx;
        }
    };

    // tests

    static_assert([]() {
        lmj::static_hash_table<int, int, 128> map;
        for (int i = 0; i < 50; ++i)
            map[i] = i;
        auto res = 0;
        for (int i = 0; i < 50; ++i)
            res += map.at(i);
        return res;
    }() == 50 * 49 / 2);
}