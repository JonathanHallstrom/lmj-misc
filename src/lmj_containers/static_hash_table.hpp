#pragma once

#include <utility>
#include <functional>
#include <cstdint>
#include "../lmj_utils/lmj_utils.hpp"
#include "container_helpers.hpp"

namespace lmj {

    template<class key_type, class value_type, std::size_t _capacity, class hash_type = lmj::hash<key_type>>
    class static_hash_table {
    public:
        using pair_type = std::pair<key_type, value_type>;
        using size_type = decltype(needed_uint<_capacity>());
        using bool_type = std::uint8_t;
        pair_type _table[_capacity]{};
        bool_type _is_set[_capacity]{};
        size_type _elem_count{};
        hash_type _hasher{};

        enum active_enum : bool_type {
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

        constexpr value_type &operator[](key_type const &_key) {
            return get(_key);
        }

        constexpr value_type const &at(key_type const &_key) const {
            size_type _idx = _get_index_read(_key);
            assert(_is_set[_idx] == active_enum::ACTIVE && "index not found");
            return _table[_idx].second;
        }

        constexpr value_type &get(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            return (_is_set[_idx] == active_enum::ACTIVE) ? _table[_idx].second : emplace(_key, value_type{});
        }

        constexpr bool contains(key_type const &_key) {
            return _is_set[_get_index_read(_key)] == active_enum::ACTIVE;
        }

        constexpr void erase(key_type const &_key) {
            remove(_key);
        }

        constexpr void remove(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            if (_is_set[_idx] == active_enum::ACTIVE) {
                --_elem_count;
                _table[_idx].~pair_type();
                _is_set[_idx] = active_enum::TOMBSTONE;
            }
        }

        constexpr value_type &insert(key_type const &_key, value_type const &_value) {
            return emplace(_key, _value);
        }

        template<class... _types>
        constexpr value_type &emplace(_types &&... _pack) {
            static_assert(sizeof...(_pack));
            auto _p = pair_type{_pack...};
            size_type _idx = _get_index_write(_p.first);
            if (_is_set[_idx] == active_enum::ACTIVE)
                return _table[_idx].second;
            ++_elem_count;
            _is_set[_idx] = active_enum::ACTIVE;
            _table[_idx].first = _p.first;
            _table[_idx].second = _p.second;
            return _table[_idx].second;
        }

        [[nodiscard]] constexpr size_type size() const {
            return _elem_count;
        }

        [[nodiscard]] constexpr size_type capacity() const {
            return _capacity;
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
            while (_is_set[_idx] == active_enum::TOMBSTONE ||
                   (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first != _key)) {
                _idx = _new_idx(_idx);
            }
            return _idx;
        }

        [[nodiscard]] constexpr size_type _get_index_write(key_type const &_key) const {
            size_type _idx = _get_hash(_key);
            while (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first != _key) {
                _idx = _new_idx(_idx);
            }
            return _idx;
        }
    };
}
