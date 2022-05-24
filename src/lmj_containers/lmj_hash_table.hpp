#pragma once

#include <cstring>
#include <utility>
#include <functional>
#include <cassert>
#include <cstdint>

namespace lmj {
    template<class key_t, class value_t, class hash_t>
    class hash_table_iterator;

    template<class key_t, class value_t, class hash_t>
    class hash_table_const_iterator;

    template<class key_type, class value_type, class hash_type = std::hash<key_type>>
    class hash_table {
        enum active_enum { // shadowing outer one so this file works on its own
            INACTIVE = 0,
            ACTIVE = 1,
            TOMBSTONE = 2,
        };
    public:
        using pair_type = std::pair<key_type const, value_type>;
        using size_type = std::size_t;
        using bool_type = std::uint8_t;
        using iterator = hash_table_iterator<key_type, value_type, hash_type>;
        using const_iterator = hash_table_const_iterator<key_type, value_type, hash_type>;
        pair_type *_table{};
        bool_type *_is_set{};
        size_type _elem_count{};
        size_type _tomb_count{};
        size_type _capacity{};
        hash_type _hasher{};


        hash_table() = default;

        constexpr hash_table(hash_table const &other) {
            *this = other;
        }

        constexpr hash_table(hash_table &&other) noexcept {
            *this = std::move(other);
        }

        constexpr explicit hash_table(hash_type _hasher) : _hasher{_hasher} {}

        explicit hash_table(size_type _size, hash_type _hasher = {}) : _hasher{_hasher} {
            _alloc_size(_size);
        }

        ~hash_table() {
            delete[] _is_set;
            delete[] _table;
        }

        hash_table &operator=(hash_table &&other) noexcept {
            if (this == &other || _table == other._table || _is_set == other._is_set)
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
            other._tomb_count = 0;
            return *this;
        }

        hash_table &operator=(hash_table const &other) {
            if (this == &other || _table == other._table || _is_set == other._is_set)
                return *this;
            _alloc_size(other._capacity);
            for (size_type i = 0; i < other._capacity; ++i) {
                if (other._is_set[i] == ACTIVE)
                    new(&_table[i]) pair_type(other._table[i]);
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
                if (_is_set[i] == ACTIVE) {
                    size_type _idx = other._get_index_read(_table[i].second);
                    if (other._is_set[_idx] != ACTIVE)
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
            assert(_capacity && "empty hash_table");
            size_type _idx = _get_index_read(_key);
            assert(_is_set[_idx] == ACTIVE && _table[_idx].first == _key && "key not found");
            return _table[_idx].second;
        }

        /**
         * @brief gets value at _key or creates new value at _key with default value
         * @return reference to value associated with _key
         */
        value_type &get(key_type const &_key) {
            if (_should_grow())
                _grow();
            size_type _idx = _get_index_read(_key);
            return (_is_set[_idx] == ACTIVE && _table[_idx].first == _key) ?
                   _table[_idx].second : emplace(_key, value_type{});
        }

        /**
         * @return whether _key is in table
         */
        bool contains(key_type const &_key) {
            if (!_capacity)
                return false;
            size_type _idx = _get_index_read(_key);
            return _is_set[_idx] == ACTIVE && _table[_idx].first == _key;
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
            if (!_capacity)
                return;
            size_type _idx = _get_index_read(_key);
            if (_is_set[_idx] == ACTIVE && _table[_idx].first == _key) {
                --_elem_count;
                ++_tomb_count;
                _table[_idx].~pair_type();
                _is_set[_idx] = TOMBSTONE;
            }
        }

        [[nodiscard]] auto begin() {
            return iterator(this, _get_start_index());
        }

        [[nodiscard]] auto end() {
            return iterator(this, _get_end_index());
        }

        [[nodiscard]] auto begin() const {
            return const_iterator(this, _get_start_index());
        }

        [[nodiscard]] auto end() const {
            return const_iterator(this, _get_end_index());
        }

        [[nodiscard]] auto cbegin() const {
            return const_iterator(this, _get_start_index());
        }

        [[nodiscard]] auto cend() const {
            return const_iterator(this, _get_end_index());
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
        template<class...G>
        value_type &emplace(G &&... _pack) {
            if (_should_grow())
                _grow();
            static_assert(sizeof...(_pack));
            auto _p = pair_type{_pack...};
            size_type _hash = _get_hash(_p.first);
            size_type _idx = _get_index_read(_p.first, _hash);
            if (_is_set[_idx] == ACTIVE && _table[_idx].first == _p.first)
                return _table[_idx].second;
            _idx = _get_writable_index(_p.first, _hash);
            ++_elem_count;
            _tomb_count -= _is_set[_idx] == TOMBSTONE;
            _is_set[_idx] = ACTIVE;
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
         * @return _table_capacity of table
         */
        [[nodiscard]] size_type capacity() const {
            return _capacity;
        }

        /**
         * @brief remove all elements
         */
        void clear() {
            for (size_type i = 0; i < _capacity; ++i) {
                if (_is_set[i] == ACTIVE) {
                    _table[i].~pair_type();
                }
                _is_set[i] = INACTIVE;
            }
            _elem_count = 0;
            _tomb_count = 0;
        }

        /**
         * @brief resizes the table and causes a rehash of all elements
         * fails if _new_capacity is less than current _table_capacity
		 * @param _new_capacity new capacity of table
         */
        void resize(size_type const _new_capacity) {
            assert(_new_capacity >= _capacity);
            hash_table _other(_new_capacity);
            for (size_type i = 0; i < _capacity; ++i) {
                if (_is_set[i] == ACTIVE)
                    _other.emplace(_table[i]);
            }
            *this = std::move(_other);
        }

        [[nodiscard]] size_type _clamp_size(size_type _idx) const {
            if ((_capacity & (_capacity - 1)) == 0)
                return _idx & (_capacity - 1);
            else
                return _idx % _capacity;
        }

    private:
        [[nodiscard]] size_type _get_start_index() const {
            if (!_capacity)
                return 0;
            for (size_type i = 0; i < _capacity; ++i)
                if (_is_set[i] == ACTIVE)
                    return i;
            return 0; // should be unreachable;
        }

        [[nodiscard]] size_type _get_end_index() const {
            return _capacity;
        }

        [[nodiscard]] size_type _get_hash(key_type const &_key) const {
            return _clamp_size(_hasher(_key));
        }

        [[nodiscard]] size_type _new_idx(size_type const _idx) const {
            return _clamp_size(_idx + 1);
        }

        [[nodiscard]] size_type _get_index_read(key_type const &_key) const {
            size_type _idx = _get_hash(_key);
            return _get_index_read_impl(_key, _idx);
        }

        [[nodiscard]] size_type _get_index_read(key_type const &_key, size_type _idx) const {
            return _get_index_read_impl(_key, _idx);
        }

        [[nodiscard]] size_type _get_index_read_impl(key_type const &_key, size_type _idx) const {
            std::size_t _iterations = 0;
            while ((_is_set[_idx] == TOMBSTONE ||
                    (_is_set[_idx] == ACTIVE && _table[_idx].first != _key))
                   && _iterations++ < _capacity) {
                _idx = _new_idx(_idx);
            }
            return _idx;
        }

        [[nodiscard]] size_type _get_writable_index(key_type const &_key) const {
            size_type _idx = _get_hash(_key);
            return _get_writable_index_impl(_key, _idx);
        }

        [[nodiscard]] size_type _get_writable_index(key_type const &_key, size_type _idx) const {
            return _get_writable_index_impl(_key, _idx);
        }

        [[nodiscard]] size_type _get_writable_index_impl(key_type const &_key, size_type _idx) const {
            std::size_t _iterations = 0;
            while (_is_set[_idx] == ACTIVE && _table[_idx].first != _key) {
                assert(_iterations++ < _capacity && "element not found");
                _idx = _new_idx(_idx);
            }
            return _idx;
        }

        [[nodiscard]] bool _should_grow() const {
            return (_elem_count + _tomb_count) * 2 >= _capacity;
        }

        void _grow() {
            constexpr auto default_size = 16;
            if (_capacity > 0)
                resize(_capacity * (2 + 6 * (_capacity < 4096)));
            else
                resize(default_size);
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

    template<class key_t, class value_t, class hash_t>
    class hash_table_iterator {
        enum active_enum { // shadowing outer one so this file works on its own
            INACTIVE = 0,
            ACTIVE = 1,
            TOMBSTONE = 2,
        };
    public:
        using pair_type = std::pair<key_t const, value_t>;
        using size_type = std::size_t;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = long long;
        using value_type = pair_type;
        using pointer = pair_type *;
        using reference = pair_type &;

        hash_table<key_t, value_t, hash_t> *const _table_ptr;
        size_type _index;

        hash_table_iterator(hash_table<key_t, value_t, hash_t> *_ptr, size_type _idx) : _table_ptr{_ptr},
                                                                                        _index{_idx} {}

        auto &operator++() {
            ++_index;
            while (_index < _table_ptr->capacity() && _table_ptr->_is_set[_index] != ACTIVE) ++_index;
            return *this;
        }

        auto &operator--() {
            --_index;
            while (_index > 0 && _table_ptr->_is_set[_index] != ACTIVE) --_index;
            return *this;
        }

        reference operator*() const {
            return _table_ptr->_table[_index];
        }

        bool operator!=(hash_table_iterator const &other) const {
            return _index != other._index || _table_ptr != other._table_ptr;
        }
    };

    template<class key_t, class value_t, class hash_t>
    class hash_table_const_iterator {
        enum active_enum { // shadowing outer one so this file works on its own
            INACTIVE = 0,
            ACTIVE = 1,
            TOMBSTONE = 2,
        };
    public:
        using pair_type = std::pair<key_t const, value_t>;
        using size_type = std::size_t;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = long long;
        using value_type = pair_type const;
        using pointer = pair_type const *;
        using reference = pair_type const &;

        hash_table<key_t, value_t, hash_t> const *const _table_ptr;
        size_type _index;

        hash_table_const_iterator(hash_table<key_t, value_t, hash_t> const *_ptr, size_type _idx) : _table_ptr{_ptr},
                                                                                                    _index{_idx} {}

        auto &operator++() {
            ++_index;
            while (_index < _table_ptr->capacity() && _table_ptr->_is_set[_index] != ACTIVE) ++_index;
            return *this;
        }

        auto &operator--() {
            --_index;
            while (_index > 0 && _table_ptr->_is_set[_index] != ACTIVE) --_index;
            return *this;
        }

        reference operator*() const {
            return _table_ptr->_table[_index];
        }

        bool operator!=(hash_table_const_iterator const &other) const {
            return _index != other._index || _table_ptr != other._table_ptr;
        }
    };
}
