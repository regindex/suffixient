#ifndef CUCKOO_HASH_HPP
#define CUCKOO_HASH_HPP

#include "Fast.hpp"
#include "dcheck.hpp"

namespace ctriepp {

#ifdef NDEBUG
#define ON_DEBUG(x)
#else
#define ON_DEBUG(x) x
#endif

#ifndef NUM_CUCKOO_HASH_FUNCTIONS
#define NUM_CUCKOO_HASH_FUNCTIONS 3
#endif

template <class map_t>
class abstract_list_iterator {
   public:
    using map_type = map_t;
    using key_type = typename map_type::key_type;
    using value_type = typename map_type::value_type;
    using node_handle_type = typename map_type::node_handle_type;

    map_type& m_map;
    size_t m_position;

    abstract_list_iterator(map_t& map, size_t position)
        : m_map(map), m_position(position) {}
    abstract_list_iterator& operator++() {
        ++m_position;
        return *this;
    }
    bool invalid() const { return m_position >= m_map.capacity(); }
    template <class U>
    bool operator!=(abstract_list_iterator<U> o) const {
        return !(this->operator==(o));
    }

    size_t position() const { return m_position; }

    template <class U>
    bool operator==(abstract_list_iterator<U> o) const {
        if (o.invalid() && invalid()) return true;
        if (!o.invalid() && !invalid()) return m_position == o.m_position;
        return false;
    }
    value_type value() const {
        DCHECK(!invalid());
        return m_map.value(m_position);
    }
    key_type key() const { return node_handle_type::key(value()); }
};

template <class node_handle_t>
class abstract_list {
   public:
    using node_handle_type = node_handle_t;
    using value_type = typename node_handle_t::index_type;
    using key_type = typename node_handle_t::key_type;  // typename T::key_type;
    using class_type = abstract_list<node_handle_t>;
    using iterator = abstract_list_iterator<class_type>;
    using const_iterator = abstract_list_iterator<const class_type>;

    // static constexpr Factory<T>& factory() {
    //     return T::factory();
    // }

    value_type* m_values = nullptr;
    size_t m_elements = 0;

    iterator begin() {
        if (m_elements == 0) return end();
        return {*this, 0};
    }
    const_iterator begin() const {
        if (m_elements == 0) return end();
        return {*this, 0};
    }
    iterator end() { return {*this, -1ULL}; }
    const_iterator end() const { return {*this, -1ULL}; }

    value_type value(const size_t position) const {
        DCHECK_LT(position, capacity());
        return m_values[position];
    }

    void swap(class_type& o) {
        std::swap(m_values, o.m_values);
        std::swap(m_elements, o.m_elements);
    }

    const size_t store(std::ofstream& out) const
    {
        size_t no_bytes = 0;

        //out.write((char*)&m_buckets,sizeof(uint_fast8_t));
        out.write((char*)&m_elements,sizeof(size_t));
        out.write((char*)m_values,capacity()*sizeof(value_type));
        no_bytes += capacity()*sizeof(value_type) + sizeof(size_t);

        return no_bytes;
    }

    const void load(std::ifstream& in)
    {
        in.read(reinterpret_cast<char*>(&m_elements),sizeof(size_t));
        in.read(reinterpret_cast<char*>(m_values),capacity()*sizeof(value_type));

        return ;
    }

    size_t size() const { return m_elements; }
    void clear() {
        if (m_values != nullptr) {
            free(m_values);
            m_values = nullptr;
        }
        m_elements = 0;
    }
    ~abstract_list() { clear(); }

    abstract_list() = default;

    size_t capacity() const { return m_elements; }

    virtual size_t locate(const key_type& key) const = 0;

    bool erase(const key_type& key) {
        const size_t position = locate(key);
        if (position >= m_elements ||
            node_handle_type::key(m_values[position]) != key)
            return false;
        for (size_t i = position + 1; i < m_elements; ++i) {
            m_values[i - 1] = m_values[i];
        }
        --m_elements;
        if (m_elements == 0) {
            clear();
        }
        return true;
    }
};

template <class node_handle_t>
class sorted_list : public abstract_list<node_handle_t> {
    using super_type = abstract_list<node_handle_t>;
    using value_type = typename super_type::value_type;
    using key_type = typename super_type::key_type;
    using class_type = typename super_type::class_type;
    using iterator = typename super_type::iterator;
    using const_iterator = typename super_type::const_iterator;
    using node_handle_type = typename super_type::node_handle_type;

    using super_type::clear;
    using super_type::m_elements;
    using super_type::m_values;

   public:
    using super_type::begin;
    using super_type::end;

    const_iterator find(const key_type& key) const {
        const size_t position = locate(key);
        if (position < m_elements &&
            node_handle_type::key(m_values[position]) == key)
            return {*this, position};
        return end();
    }

    virtual size_t locate(const key_type& key) const override {
#ifndef NDEBUG
        size_t ret = m_elements;
        for (size_t position = 0; position < m_elements; ++position) {
            if (node_handle_type::key(m_values[position]) >= key) {
                ret = position;
                break;
            }
        }
        for (size_t i = 1; i < m_elements; ++i) {
            DCHECK_LT(node_handle_type::key(m_values[i - 1]),
                      node_handle_type::key(m_values[i]));
        }
        std::vector<key_type> v;
        for (size_t i = 0; i < m_elements; ++i) {
            v.push_back(node_handle_type::key(m_values[i]));
        }
#endif  // NDEBUG

        size_t left = 0;
        size_t right = m_elements;
        while (left + 1 < right) {
            const size_t med = (left + right) / 2;
            const key_type medKey = node_handle_type::key(m_values[med]);
            if (medKey == key) {
                break;
            }
            if (medKey < key) {
                left = med + 1;
                DCHECK_LE(left, ret);
            } else {
                right = med;
                DCHECK_LE(ret, med);
            }
        }
        const size_t med = (left + right) / 2;
#ifndef NDEBUG
        if (ret < m_elements) {
            if (v[ret] == key)
                DCHECK_EQ(ret, med);
            else {
                if (med > 0) DCHECK_LT(v[med - 1], key);
                if (med + 1 < m_elements) DCHECK_LT(key, v[med + 1]);
            }
        }
#endif
        return med;
    }

    void insert(const value_type& value) {
        const key_type key = node_handle_type::key(value);
        size_t position = locate(key);
        if (position < m_elements) {
            const key_type position_key =
                node_handle_type::key(m_values[position]);
            if (position_key == key) return;  // already inserted
            if (position_key < key) ++position;
        }

        ++m_elements;
        m_values = reinterpret_cast<value_type*>(
            realloc(m_values, sizeof(value_type) * m_elements));

        for (size_t i = m_elements - 1; i > position; --i) {
            m_values[i] = m_values[i - 1];
        }

        m_values[position] = value;
    }
};

template <class node_handle_t>
class unsorted_list : public abstract_list<node_handle_t> {
    using class_type = unsorted_list<node_handle_t>;
    using super_type = abstract_list<node_handle_t>;

    using value_type = typename super_type::value_type;
    using key_type = typename super_type::key_type;
    using iterator = typename super_type::iterator;
    using const_iterator = typename super_type::const_iterator;
    using node_handle_type = typename super_type::node_handle_type;

    using super_type::clear;
    using super_type::m_elements;
    using super_type::m_values;

   public:
    using super_type::begin;
    using super_type::end;

    const_iterator find(const key_type& key) const {
        const size_t position = locate(key);
        if (position < m_elements) return {*this, position};
        return end();
    }

    size_t locate(const key_type& key) const {
        for (size_t position = 0; position < m_elements; ++position) {
            const key_type pos_key = node_handle_type::key(m_values[position]);
            if (pos_key == key) return position;
        }
        return -1ULL;
    }

    void insert(const value_type& value) {
        const key_type key = node_handle_type::key(value);
        const size_t position = locate(key);
        if (position < m_elements &&
            node_handle_type::key(m_values[position]) == key)
            return;  // already inserted

        ++m_elements;
        m_values = reinterpret_cast<value_type*>(
            realloc(m_values, sizeof(value_type) * m_elements));
        m_values[m_elements - 1] = value;
    }
};

template <class map_t>
class cuckoo_hash_iterator {
   public:
    using map_type = map_t;
    using key_type = typename map_type::key_type;
    using value_type = typename map_type::value_type;
    using node_handle_type = typename map_type::node_handle_type;

    map_type& m_map;
    size_t m_position;

    size_t position() const { return m_position; }

    cuckoo_hash_iterator(map_t& map, size_t position)
        : m_map(map), m_position(position) {}
    cuckoo_hash_iterator& operator++() {
        do {
            ++m_position;
        } while (m_position < m_map.capacity() &&
                 m_map.value(m_position) == m_map.empty_value());
        return *this;
    }
    bool invalid() const {
        return m_position >= m_map.capacity() ||
               m_map.value(m_position) == m_map.empty_value();
    }
    template <class U>
    bool operator!=(cuckoo_hash_iterator<U> o) const {
        return !(this->operator==(o));
    }

    template <class U>
    bool operator==(cuckoo_hash_iterator<U> o) const {
        if (o.invalid() && invalid()) return true;
        if (!o.invalid() && !invalid()) return m_position == o.m_position;
        return false;
    }
    value_type value() const {
        DCHECK(!invalid());
        return m_map.value(m_position);
    }
    key_type key() const { return node_handle_type::key(value()); }
};

class cuckoo_hash_function {
   private:
    uint64_t seed_a;
    uint64_t seed_b;

   public:
    cuckoo_hash_function()
        : seed_a(((static_cast<uint64_t>(rand()) << 32) | rand()) | 1ULL),
          seed_b(((static_cast<uint64_t>(rand()) << 32) | rand()) | 1ULL) {}

    uint64_t operator()(uint64_t h) const {  // based from murmur64
        h ^= h >> 33;
        h *= seed_a;
        h ^= h >> 33;
        h *= seed_b;
        h ^= h >> 33;
        return h;
    }
};

template <class node_handle_t>
class cuckoo_hash {
   public:
    using class_type = cuckoo_hash<node_handle_t>;

    using node_handle_type = node_handle_t;
    using value_type = typename node_handle_type::index_type;
    using key_type = typename node_handle_type::key_type;

    using iterator = cuckoo_hash_iterator<class_type>;
    using const_iterator = cuckoo_hash_iterator<const class_type>;

    value_type* m_values = nullptr;
    uint_fast8_t m_buckets = 0;
    size_t m_elements = 0;

    static cuckoo_hash_function m_hash_a;
    static cuckoo_hash_function m_hash_b;
    #if NUM_CUCKOO_HASH_FUNCTIONS == 3
    static cuckoo_hash_function m_hash_c;
    #endif  // NUM_CUCKOO_HASH_FUNCTIONS == 3

    const value_type m_empty_value;

    // STORE AND LOAD NOT YET FINISHED
    const size_t store(std::ofstream& out) const
    {
        size_t no_bytes = 0;

        out.write((char*)&m_buckets,sizeof(uint_fast8_t));
        out.write((char*)&m_elements,sizeof(size_t));
        out.write((char*)m_values,capacity()*sizeof(value_type));
        no_bytes += capacity()*sizeof(value_type) + sizeof(uint_fast8_t) + sizeof(size_t);

        return no_bytes;
    }
    const void load(std::ifstream& in)
    {
        //std::cout << "1.1.1" << std::endl;
        in.read(reinterpret_cast<char*>(&m_buckets),sizeof(uint_fast8_t));
        //std::cout << "1.1.2" << std::endl;
        in.read(reinterpret_cast<char*>(&m_elements),sizeof(size_t));
        //std::cout << "1.1.3 " << capacity() << std::endl;
        m_values = new value_type(capacity());
        in.read(reinterpret_cast<char*>(m_values),capacity()*sizeof(value_type));
        //std::cout << "1.1.4" << std::endl;
        //exit(1);

        return ;
    }

    iterator begin() {
        if (m_buckets == 0) return end();
        const size_t cbucket_count = capacity();
        for (size_t bucket = 0; bucket < cbucket_count; ++bucket) {
            if (m_values[bucket] != m_empty_value) return {*this, bucket};
        }
        return end();
    }
    const_iterator begin() const {
        if (m_buckets == 0) return end();
        const size_t cbucket_count = capacity();
        for (size_t bucket = 0; bucket < cbucket_count; ++bucket) {
            if (m_values[bucket] != m_empty_value) return {*this, bucket};
        }
        return end();
    }
    iterator end() { return {*this, -1ULL}; }
    const_iterator end() const { return {*this, -1ULL}; }

    value_type value(const size_t position) const {
        DCHECK_LT(position, capacity());
        // DCHECK_NE(m_values[position], m_empty_value);
        return m_values[position];
    }

    void swap(class_type& o) {
        std::swap(m_values, o.m_values);
        std::swap(m_buckets, o.m_buckets);
        std::swap(m_elements, o.m_elements);
        // std::swap(m_hash_a, o.m_hash_a);
        // std::swap(m_hash_b, o.m_hash_b);
        // std::swap(m_hash_c, o.m_hash_c);
    }

    const value_type& empty_value() const { return m_empty_value; }

    size_t size() const { return m_elements; }
    void clear() {
        if (m_values != nullptr) {
            free(m_values);
            m_values = nullptr;
        }
        m_elements = 0;
    }
    ~cuckoo_hash() { clear(); }

    cuckoo_hash(const size_t bucket_count, const value_type empty_value)
        : m_empty_value(empty_value) {
        if (bucket_count > 0) {
            reserve(bucket_count);
        }
    }

    size_t capacity() const { return 1ULL << m_buckets; }

    void reserve(size_t reserve) {
        size_t reserve_bits = Fast::mostSignificantBit(reserve);
        if (1ULL << reserve_bits != reserve) ++reserve_bits;
        const size_t new_size = 1ULL << reserve_bits;
        if (m_buckets == 0) {
            m_values = reinterpret_cast<value_type*>(
                malloc(new_size * sizeof(value_type)));
            std::fill(m_values, m_values + new_size, m_empty_value);
            m_buckets = reserve_bits;
        } else {
            class_type tmp_map(new_size, m_empty_value);
        #if STATS_ENABLED
            tdc::StatPhase statphase(std::string("resizing to ") +
                                     std::to_string(reserve_bits));
            print_stats(statphase);
        #endif
            const size_t cbucket_count = capacity();
            for (size_t bucket = 0; bucket < cbucket_count; ++bucket) {
                const value_type& value = m_values[bucket];
                if (value == m_empty_value) continue;
                tmp_map.insert(value);
            }
            swap(tmp_map);
        }
    }
    static float m_load_factor;

    static float max_load_factor() { return m_load_factor; }
    static void max_load_factor(float ml) { m_load_factor = ml; }

    const_iterator find(const key_type& key) const {
        if (m_buckets == 0) return end();

        for (size_t i = 0; i < NUM_CUCKOO_HASH_FUNCTIONS; i++) {
            const size_t position = get_position(key, i);
            if (m_values[position] != m_empty_value) {
                const key_type position_key =
                    node_handle_type::key(m_values[position]);
                if (position_key == key) {
                    return const_iterator{*this, position};
                }
            }
        }

        return end();
    }

    bool erase(const key_type& key) {
        if (m_buckets == 0) return false;

        for (size_t i = 0; i < NUM_CUCKOO_HASH_FUNCTIONS; i++) {
            const size_t position = get_position(key, i);
            if (m_values[position] != m_empty_value) {
                const key_type position_key =
                    node_handle_type::key(m_values[position]);
                if (position_key == key) {
                    m_values[position] = m_empty_value;
                    --m_elements;
                    return true;
                }
            }
        }
        return false;
    }

    void insert(const value_type& value) {
        assert(find(node_handle_type::key(value)) == end());
        const key_type key = node_handle_type::key(value);
        insert(value, 0);
        assert(find(node_handle_type::key(value)) != end());
    }

   private:
    constexpr static size_t MAX_TRIES =
        100;  // number cuckoo replacement steps before considering this as an
              // endless loop -> resort to rehashing
    void insert(const value_type& value, const size_t tries,
                const size_t prev_id = -1) {
        const key_type key = node_handle_type::key(value);
        if (m_elements + 1 > capacity() * max_load_factor() ||
            tries > capacity() || tries > MAX_TRIES) {
            reserve(capacity() << 1);
        }

        size_t next_id;
        if (prev_id == -1) {
            next_id = rand() % NUM_CUCKOO_HASH_FUNCTIONS;
        } else {
            // next_id other than prev_id
            next_id =
                ((rand() % (NUM_CUCKOO_HASH_FUNCTIONS - 1)) + prev_id + 1) %
                NUM_CUCKOO_HASH_FUNCTIONS;
        }
        size_t next_position;

        for (size_t i = 0; i < NUM_CUCKOO_HASH_FUNCTIONS; i++) {
            if (i == prev_id) {
                continue;
            }

            const size_t position = get_position(key, i);
            if (i == next_id) {
                next_position = position;
            }
            if (m_values[position] == m_empty_value) {
                m_values[position] = value;
                ++m_elements;
                return;
            }
            const key_type position_key =
                node_handle_type::key(m_values[position]);
            if (position_key == key) {
                m_values[position] = value;
                return;
            }
        }

        const value_type tmp_value = m_values[next_position];
        m_values[next_position] = value;
        insert(tmp_value, tries + 1, next_id);
    }

    uint64_t get_position(uint64_t key, size_t hash_id) const {
        uint64_t hash_value;
        switch (hash_id) {
            case 1: {
                hash_value = m_hash_b(key);
                break;
            }
#if NUM_CUCKOO_HASH_FUNCTIONS == 3
            case 2: {
                hash_value = m_hash_c(key);
                break;
            }
#endif
            default: {
                hash_value = m_hash_a(key);
                break;
            }
        }
        return hash_value & (-1ULL >> (64 - m_buckets));
    }
};

template <class node_handle_t>
float cuckoo_hash<node_handle_t>::m_load_factor = 0.9f;
template <class node_handle_t>
cuckoo_hash_function cuckoo_hash<node_handle_t>::m_hash_a;
template <class node_handle_t>
cuckoo_hash_function cuckoo_hash<node_handle_t>::m_hash_b;
#if NUM_CUCKOO_HASH_FUNCTIONS == 3
template <class node_handle_t>
cuckoo_hash_function cuckoo_hash<node_handle_t>::m_hash_c;
#endif  // NUM_CUCKOO_HASH_FUNCTIONS == 3

}  // namespace ctriepp

namespace std {
template <class map_t>
std::string to_string(const ctriepp::abstract_list_iterator<map_t>& it) {
    std::stringstream ss;
    ss << "position: " << it.position() << "; ";
    return ss.str();
}
}  // namespace std

namespace std {
template <class map_t>
std::string to_string(const ctriepp::cuckoo_hash_iterator<map_t>& it) {
    std::stringstream ss;
    ss << "position: " << it.position() << "; ";
    return ss.str();
}
}  // namespace std

#endif  // CUCKOO_HASH_HPP
