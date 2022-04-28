#pragma once

#include <cstring>
#include <utility>
#include <functional>
#include <cassert>
#include <cstdint>

namespace lmj {
    template<class key_type, class value_type, class hash_type = std::hash<key_type>>
    class hash_table {
    public:
        using pair_type = std::pair<const key_type, value_type>;
        using size_type = std::size_t;
        using bool_type = std::uint8_t;
        pair_type *_table{};
        bool_type *_is_set{};
        size_type _elem_count{};
        size_type _tomb_count{};
        size_type _capacity{};
        hash_type _hasher{};

        enum active_enum : bool_type {
            ACTIVE = 1,
            TOMBSTONE = 2
        };

        hash_table() {
            _alloc_size(128);
        }

        hash_table(hash_table const &other) {
            *this = other;
        }

        hash_table(hash_table &&other) noexcept {
            *this = std::move(other);
        }

        explicit hash_table(hash_type _hasher) : _hasher(_hasher) {
            _alloc_size(128);
        }

        explicit hash_table(size_type _size) {
            _alloc_size(_size);
        }

        ~hash_table() {
            delete[] _is_set;
            delete[] _table;
        }

        hash_table &operator=(hash_table &&other) noexcept {
            if (this == &other || this->_table == other._table || this->_is_set == other._is_set)
                return *this;
            delete[] _table;
            delete[] _is_set;
            _table = other._table;
            _is_set = other._is_set;
            _elem_count = other._elem_count;
            _capacity = other._capacity;
            _hasher = other._hasher;
            _tomb_count = other._tomb_count;
            other._is_set = nullptr;
            other._table = nullptr;
            other._elem_count = 0;
            other._capacity = 0;
            return *this;
        }

        hash_table &operator=(hash_table const &other) {
            if (this == &other || this->_table == other._table || this->_is_set == other._is_set)
                return *this;
            _alloc_size(other._capacity);
            for (size_type i = 0; i < other._capacity; ++i) {
                if (other._is_set[i] == active_enum::ACTIVE) {
                    new(&_table[i]) pair_type(other._table[i]);
                }
                _is_set[i] = other._is_set[i];
            }
            _tomb_count = 0;
            _elem_count = other._elem_count;
            _capacity = other._capacity;
            _hasher = other._hasher;
            return *this;
        }

        bool operator==(hash_table const &other) const {
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
        value_type &operator[](key_type const &_key) {
            return get(_key);
        }

        /**
         * @return value at _key or fails
         */
        value_type const &at(key_type const &_key) const {
            size_type _idx = _get_index_read(_key);
            assert(_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _key && "key not found");
            return _table[_idx].second;
        }

        /**
         * @brief gets value at _key or creates new value at _key with default value
         * @return reference to value associated with _key
         */
        value_type &get(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            return (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _key) ?
                   _table[_idx].second : emplace(_key, value_type{});
        }

        /**
         * @return whether _key is in table
         */
        bool contains(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            return _is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _key;
        }

        /**
         * @param _key key which is removed from table
         */
        void erase(key_type const &_key) {
            remove(_key);
        }

        /**
         * @param _key key which is removed from table
         */
        void remove(key_type const &_key) {
            size_type _idx = _get_index_read(_key);
            if (_is_set[_idx] == active_enum::ACTIVE) {
                --_elem_count;
                ++_tomb_count;
                _table[_idx].~pair_type();
                _is_set[_idx] = active_enum::TOMBSTONE;
            }
        }

        /**
         * @param _key
         * @param _value
         * @return reference to _value in table
         */
        value_type &insert(key_type const &_key, value_type const &_value) {
            return emplace(_key, _value);
        }

        /**
         * @param _pack arguments for constructing element
         * @return  reference to newly constructed value
         */
        value_type &emplace(auto &&... _pack) {
            if (_should_grow())
                _grow();
            static_assert(sizeof...(_pack));
            auto _p = pair_type{_pack...};
            size_type _idx = _get_index_read(_p.first);
            if (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first == _p.first)
                return _table[_idx].second;
            _idx = _get_writable_index(_p.first);
            ++_elem_count;
            _tomb_count -= _is_set[_idx] == active_enum::TOMBSTONE;
            _is_set[_idx] = active_enum::ACTIVE;
            new(_table + _idx) pair_type(_p);
            return _table[_idx].second;
        }

        /**
         * @return number of elements
         */
        [[nodiscard]] size_type size() const {
            return _elem_count;
        }

        /**
         * @return capacity of table
         */
        [[nodiscard]] size_type capacity() const {
            return _capacity;
        }

    private:
        void _resize(size_type const _new_capacity) {
            hash_table _other(_new_capacity);
            for (size_type i = 0; i < _capacity; ++i) {
                _other.emplace(_table[i]);
            }
            *this = move(_other);
            assert(_new_capacity == _capacity);
        }

        [[nodiscard]] size_type _clamp_size(size_type _idx) const {
            if ((_capacity & (_capacity - 1)) == 0)
                return _idx & (_capacity - 1);
            else
                return _idx % _capacity;
        }

        [[nodiscard]] size_type _get_hash(key_type const &_key) const {
            return _clamp_size(_hasher(_key));
        }

        [[nodiscard]] size_type _new_idx(size_type const _idx) const {
            return _clamp_size(_idx + 1);
        }

        [[nodiscard]] size_type _get_index_read(key_type const &_key) const {
            size_type _idx = _get_hash(_key);
            std::size_t _iterations = 0;
            while (_is_set[_idx] == active_enum::TOMBSTONE ||
                   (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first != _key)) {
                if (_iterations++ == _capacity)
                    return _idx;
                _idx = _new_idx(_idx);
            }
            return _idx;
        }

        [[nodiscard]] size_type _get_writable_index(key_type const &_key) const {
            size_type _idx = _get_hash(_key);
            std::size_t _iterations = 0;
            while (_is_set[_idx] == active_enum::ACTIVE && _table[_idx].first != _key) {
                assert(_iterations++ < _capacity && "element not found");
                _idx = _new_idx(_idx);
            }
            return _idx;
        }

        [[nodiscard]] bool _should_grow() const {
            return (_elem_count + _tomb_count) * 3 / 2 > _capacity;
        }

        void _grow() {
            _resize((_capacity + !_capacity) * 2);
        }

        void _alloc_size(size_type const _new_capacity) {
            delete[] _is_set;
            delete[] _table;
            _is_set = new bool_type[_new_capacity]{};
            _table = new pair_type[_new_capacity];
            _elem_count = 0;
            _tomb_count = 0;
            _capacity = _new_capacity;
        }
    };
}
