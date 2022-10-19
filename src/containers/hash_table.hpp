#pragma once

#include <cstring>
#include <utility>
#include <functional>
#include <cassert>
#include <cstdint>

namespace lmj {
namespace detail {
template<class T>
constexpr auto next_power_of_two_inclusive(T x) {
    T result = 1;
    while (result < x) {
        result *= 2;
    }
    return result;
}
}

template<class key_t, class value_t, class hash_t>
class hash_table_iterator;

template<class key_t, class value_t, class hash_t>
class hash_table_const_iterator;

template<class key_type, class value_type, class hash_type = std::hash<key_type>>
class hash_table {
    enum active_enum {
        INACTIVE = 0,
        ACTIVE = 1,
        TOMBSTONE = 2,
    };
public:
    using pair_type = std::pair<const key_type, value_type>;
    using size_type = std::size_t;
    using bool_type = std::uint8_t;
    using iterator = hash_table_iterator<key_type, value_type, hash_type>;
    using const_iterator = hash_table_const_iterator<key_type, value_type, hash_type>;
    pair_type *m_table{};
    bool_type *m_is_set{};
    size_type m_elem_count{};
    size_type m_tomb_count{};
    size_type m_capacity{};
    hash_type m_hasher{};

    hash_table() = default;

    hash_table(hash_table const &other) {
        *this = other;
    }

    hash_table(hash_table &&other) noexcept {
        *this = std::move(other);
    }

    hash_table(std::initializer_list<pair_type> l) {
        for (auto &p: l) emplace(std::move(p));
    }

    explicit hash_table(hash_type hasher) : m_hasher{hasher} {}

    explicit hash_table(size_type size, hash_type hasher = {}) : m_hasher{hasher} { _alloc_size(size); }

    ~hash_table() {
        delete[] m_is_set;
        delete[] m_table;
    }

    hash_table &operator=(hash_table &&other) noexcept {
        if (this == &other || m_table == other.m_table || m_is_set == other.m_is_set)
            return *this;
        delete[] m_table;
        delete[] m_is_set;
        m_table = other.m_table;
        m_is_set = other.m_is_set;
        m_elem_count = other.m_elem_count;
        m_capacity = other.m_capacity;
        if constexpr (std::is_copy_assignable_v<hash_type>)
            m_hasher = other.m_hasher;
        m_tomb_count = other.m_tomb_count;
        other.m_is_set = nullptr;
        other.m_table = nullptr;
        other.m_elem_count = 0;
        other.m_capacity = 0;
        other.m_tomb_count = 0;
        return *this;
    }

    hash_table &operator=(hash_table const &other) {
        if (this == &other || m_table == other.m_table || m_is_set == other.m_is_set)
            return *this;
        _alloc_size(other.m_capacity);
        for (size_type i = 0; i < other.m_capacity; ++i) {
            if (other.m_is_set[i] == ACTIVE) {
                new(&m_table[i]) pair_type{other.m_table[i]};
                m_is_set[i] = other.m_is_set[i];
            }
        }
        m_tomb_count = 0;
        m_elem_count = other.m_elem_count;
        m_capacity = other.m_capacity;
        if constexpr (std::is_copy_assignable_v<hash_type>)
            m_hasher = other.m_hasher;
        return *this;
    }

    template<class hash_t>
    bool operator==(hash_table<key_type, value_type, hash_t> const &other) const {
        if (other.size() != this->size())
            return false;
        for (size_type i = 0; i < m_capacity; ++i) {
            if (m_is_set[i] == ACTIVE &&
                other.contains(m_table[i].first) &&
                other.at(m_table[i].first) != m_table[i].second) {
                return false;
            }
        }
        return true;
    }

    /**
     * @return reference to value associated with key or default constructs value if it doesn't exist
     */
    value_type &operator[](key_type const &key) {
        return get(key);
    }

    /**
     * @return value at key or fails
     */
    value_type const &at(key_type const &key) const {
        assert(m_capacity && "empty hash_table");
        const size_type idx = _get_index_read(key);
        assert(m_is_set[idx] == ACTIVE && m_table[idx].first == key && "key not found");
        return m_table[idx].second;
    }

    /**
     * @brief gets value at key or creates new value at key with default value
     * @return reference to value associated with key
     */
    value_type &get(key_type const &key) {
        if (!m_capacity || !m_elem_count)
            return emplace(key, value_type{});
        const size_type idx = _get_index_read(key);
        return (m_is_set[idx] == ACTIVE && m_table[idx].first == key) ?
               m_table[idx].second : emplace(key, value_type{});
    }

    /**
     * @return whether key is in table
     */
    bool contains(key_type const &key) const {
        if (!m_elem_count)
            return false;
        const size_type idx = _get_index_read(key);
        return m_is_set[idx] == ACTIVE && m_table[idx].first == key;
    }

    /**
     * @param _key key which is removed from table
     */
    void erase(key_type const &_key) {
        remove(_key);
    }

    /**
     * @param key key which is removed from table
     */
    void remove(key_type const &key) {
        if (!m_elem_count)
            return;
        const size_type idx = _get_index_read(key);
        if (m_is_set[idx] == ACTIVE && m_table[idx].first == key) {
            --m_elem_count;
            ++m_tomb_count;
            m_table[idx].~pair_type();
            m_is_set[idx] = TOMBSTONE;
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
     * @param pair
     * @return reference to _value in table
     */
    value_type &insert(pair_type const &pair) {
        return emplace(pair);
    }

    /**
     * @param args arguments for constructing element
     * @return  reference to newly constructed value
     */
    template<class...Args>
    value_type &emplace(Args &&... args) {
        if (_should_grow())
            _grow();
        static_assert(sizeof...(args));
        auto p = pair_type{std::forward<Args>(args)...};
        const size_type hash = _get_hash(p.first);
        size_type idx = _get_index_read(p.first, hash);
        if (m_is_set[idx] == ACTIVE && m_table[idx].first == p.first)
            return m_table[idx].second;
        idx = _get_writable_index(p.first, hash);
        ++m_elem_count;
        m_tomb_count -= m_is_set[idx] == TOMBSTONE;
        m_is_set[idx] = ACTIVE;
        new(m_table + idx) pair_type{std::move(p)};
        return m_table[idx].second;
    }

    /**
     * @return number of elements
     */
    [[nodiscard]] size_type size() const {
        return m_elem_count;
    }

    /**
     * @return _table_capacity of table
     */
    [[nodiscard]] size_type capacity() const {
        return m_capacity;
    }

    /**
     * @brief remove all elements
     */
    void clear() {
        for (size_type i = 0; i < m_capacity; ++i) {
            if (m_is_set[i] == ACTIVE) {
                m_table[i].~pair_type();
            }
            m_is_set[i] = INACTIVE;
        }
        m_elem_count = 0;
        m_tomb_count = 0;
    }

    /**
     * @brief resizes the table and causes a rehash of all elements
     * fails if new_capacity is less than current number of elements
     * @param new_capacity new capacity of table
     */
    void resize(size_type const new_capacity) {
        assert(new_capacity >= m_elem_count);
        hash_table other{new_capacity, m_hasher};
        for (size_type i = 0; i < m_capacity; ++i) {
            if (m_is_set[i] == ACTIVE)
                other.emplace(m_table[i]);
        }
        *this = std::move(other);
    }

    const_iterator find(key_type const &key) const {
        if (!m_elem_count)
            return end();
        const size_type idx = _get_index_read(key);
        if (m_is_set[idx] == ACTIVE && m_table[idx].first == key)
            return const_iterator(this, idx);
        return end();
    }

    iterator find(key_type const &key) {
        if (!m_elem_count)
            return end();
        const size_type idx = _get_index_read(key);
        if (m_is_set[idx] == ACTIVE && m_table[idx].first == key)
            return iterator(this, idx);
        return end();
    }

    [[nodiscard]] size_type _clamp_size(size_type idx) const {
        if (m_capacity & (m_capacity - 1))
            return idx % m_capacity;
        else
            return idx & (m_capacity - 1);
    }

    /**
     * @note resize but assumes new size fits all elements
     * @note don't use this without reading the implementation
     */
    void _set_size(size_type new_size) {
        hash_table other;
        other._alloc_size(new_size);
        for (size_type i = 0; i < m_capacity; ++i) {
            if (m_is_set[i] == ACTIVE) {
                const size_type idx = other._get_writable_index(m_table[i].first);
                new(&other.m_table[idx]) pair_type{m_table[i]};
                other.m_is_set[idx] = ACTIVE;
            }
        }
        *this = std::move(other);
    }

private:
    [[nodiscard]] size_type _get_start_index() const {
        if (!m_capacity)
            return 0;
        for (size_type i = 0; i < m_capacity; ++i)
            if (m_is_set[i] == ACTIVE)
                return i;
        return 0; // should be unreachable;
    }

    [[nodiscard]] size_type _get_end_index() const {
        return m_capacity;
    }

    [[nodiscard]] size_type _get_hash(key_type const &key) const {
        return _clamp_size(m_hasher(key));
        const size_type hash = m_hasher(key);
        return _clamp_size(hash ^ (~hash >> 16) ^ (hash << 24));
    }

    [[nodiscard]] size_type _new_idx(size_type const idx) const {
        return _clamp_size(idx + 1);
    }

    [[nodiscard]] size_type _get_index_read(key_type const &key) const {
        return _get_index_read_impl(key, _get_hash(key));
    }

    [[nodiscard]] size_type _get_index_read(key_type const &key, size_type idx) const {
        return _get_index_read_impl(key, idx);
    }

    [[nodiscard]] size_type _get_index_read_impl(key_type const &key, size_type idx) const {
        std::size_t iterations = 0;
        while ((m_is_set[idx] == TOMBSTONE ||
                (m_is_set[idx] == ACTIVE && m_table[idx].first != key)) && iterations++ < m_capacity) {
            idx = _new_idx(idx);
        }
        return idx;
    }

    [[nodiscard]] size_type _get_writable_index(key_type const &key) const {
        return _get_writable_index_impl(key, _get_hash(key));
    }

    [[nodiscard]] size_type _get_writable_index(key_type const &key, size_type idx) const {
        return _get_writable_index_impl(key, idx);
    }

    [[nodiscard]] size_type _get_writable_index_impl(key_type const &key, size_type idx) const {
        std::size_t iterations = 0;
        while (m_is_set[idx] == ACTIVE && m_table[idx].first != key) {
            assert(iterations++ < m_capacity && "element not found");
            idx = _new_idx(idx);
        }
        return idx;
    }

    [[nodiscard]] bool _should_grow() const {
        return !m_capacity || (m_elem_count + m_tomb_count) * 2 > m_capacity;
    }

    void _grow() {
        constexpr auto default_size = 1;
        if (m_capacity == 0) {
            resize(default_size);
        } else {
            const size_type pow2 = detail::next_power_of_two_inclusive(m_capacity);
            const size_type new_capacity = pow2 < 4096 ? std::min<size_type>(pow2 * 8, 8192) : pow2 * 2;
            resize(new_capacity);
        }
    }

    void _alloc_size(size_type new_capacity) {
        delete[] m_is_set;
        delete[] m_table;
        m_is_set = new bool_type[new_capacity]{};
        m_table = new pair_type[new_capacity];
        m_elem_count = 0;
        m_tomb_count = 0;
        m_capacity = new_capacity;
    }
};

template<class key_t, class value_t, class hash_t>
class hash_table_iterator {
    enum active_enum {
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

    hash_table<key_t, value_t, hash_t> *const m_table_ptr;
    size_type m_index;

    hash_table_iterator(hash_table<key_t, value_t, hash_t> *ptr, size_type idx) : m_table_ptr{ptr},
                                                                                  m_index{idx} {}

    auto &operator++() {
        ++m_index;
        while (m_index < m_table_ptr->capacity() && m_table_ptr->m_is_set[m_index] != ACTIVE) ++m_index;
        return *this;
    }

    auto &operator--() {
        --m_index;
        while (m_index > 0 && m_table_ptr->m_is_set[m_index] != ACTIVE) --m_index;
        return *this;
    }

    reference operator*() const {
        return m_table_ptr->m_table[m_index];
    }

    auto operator->() const {
        return &m_table_ptr->m_table[m_index];
    }

    template<class T>
    bool operator!=(T const &other) const {
        return m_index != other.m_index || m_table_ptr != other.m_table_ptr;
    }

    template<class T>
    bool operator==(T other) const {
        return m_index == other.m_index || m_table_ptr == other.m_table_ptr;
    }
};

template<class key_t, class value_t, class hash_t>
class hash_table_const_iterator {
    enum active_enum {
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

    hash_table<key_t, value_t, hash_t> const *const m_table_ptr;
    size_type m_index;

    hash_table_const_iterator(hash_table<key_t, value_t, hash_t> const *ptr, size_type idx) :
            m_table_ptr{ptr}, m_index{idx} {}

    hash_table_const_iterator(hash_table_iterator<key_t, value_t, hash_t> const &other) :
            m_table_ptr{other.m_table_ptr}, m_index{other.m_index} {}

    auto &operator++() {
        ++m_index;
        while (m_index < m_table_ptr->capacity() && m_table_ptr->m_is_set[m_index] != ACTIVE) ++m_index;
        return *this;
    }

    auto &operator--() {
        --m_index;
        while (m_index > 0 && m_table_ptr->m_is_set[m_index] != ACTIVE) --m_index;
        return *this;
    }

    reference operator*() const {
        return m_table_ptr->m_table[m_index];
    }

    auto operator->() const {
        return &m_table_ptr->m_table[m_index];
    }

    template<class T>
    bool operator!=(T const &other) const {
        return m_index != other.m_index || m_table_ptr != other.m_table_ptr;
    }

    template<class T>
    bool operator==(T other) const {
        return m_index == other.m_index || m_table_ptr == other.m_table_ptr;
    }
};
}
