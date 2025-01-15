#include "LongString.hpp"
#include "dcheck.hpp"

#ifdef NDEBUG
#define ON_DEBUG(x)
#else
#define ON_DEBUG(x) x
#endif

template<class map_t>
class abstract_list_iterator {
   public:
   using map_type = map_t;
   using key_type = typename map_type::key_type;
   using value_type = typename map_type::value_type;
   using node_handle_type = typename map_type::node_handle_type;

   map_type& m_map;
   size_t m_position;

   abstract_list_iterator(map_t& map, size_t position) 
      : m_map(map), m_position(position) 
   { }
   abstract_list_iterator& operator++() {
      ++m_position;
      return *this;
   }
   bool invalid() const {
      return m_position >= m_map.capacity();
   }
   template<class U>
   bool operator!=(abstract_list_iterator<U> o) const { return ! (this->operator==(o)); }

   size_t position() const { return m_position; }

   template<class U>
   bool operator==(abstract_list_iterator<U> o) const {
      if( o.invalid() && invalid()) return true;
      if(!o.invalid() && !invalid()) return m_position == o.m_position;
      return false;
   }
   value_type value() const {
      DCHECK(!invalid());
      return m_map.value(m_position);
   }
   key_type key() const {
      return node_handle_type::key(value());
   }
};

namespace std {
template<class map_t>
std::string to_string(const abstract_list_iterator<map_t>& it)  {
   std::stringstream ss;
   ss << "position: " << it.position() << "; ";
   return ss.str();
}
}


template<class node_handle_t>
class abstract_list {
   public:
   using node_handle_type = node_handle_t;
   using value_type = typename node_handle_t::index_type;
   using key_type = typename node_handle_t::key_type; //typename T::key_type;
   using class_type = abstract_list<node_handle_t>;
   using iterator = abstract_list_iterator<class_type>;
   using const_iterator = abstract_list_iterator<const class_type>;

   // static constexpr Factory<T>& factory() {
   //    return T::factory();
   // }

   value_type* m_values = nullptr;
   size_t m_elements = 0;


   iterator begin() {
      if(m_elements == 0) return end();
      return { *this, 0 };
   }
   const_iterator begin() const {
      if(m_elements == 0) return end();
      return { *this, 0 };
   }
   iterator end() {
      return {*this, -1ULL };
   }
   const_iterator end() const {
      return {*this, -1ULL };
   }

   value_type value(const size_t position) const {
      DCHECK_LT(position, capacity());
      return m_values[position];
   }

   void swap(class_type& o) {
      std::swap(m_values, o.m_values);
      std::swap(m_elements, o.m_elements);
   }

   size_t size() const {
      return m_elements;
   }
   void clear() {
      if(m_values != nullptr) {
         free(m_values);
         m_values = nullptr;
      }
      m_elements = 0;
   }
   ~abstract_list() { clear(); }
   
   abstract_list() = default;

   size_t capacity() const {
      return m_elements;
   }


   virtual size_t locate(const key_type& key) const = 0;

   bool erase(const key_type& key) {
      const size_t position = locate(key);
      if(position >= m_elements || node_handle_type::key(m_values[position]) != key) return false;
      for(size_t i = position+1; i < m_elements; ++i) {
         m_values[i-1] = m_values[i];
      }
      --m_elements;
      if(m_elements == 0) {
         clear();
      }
      return true;
   }

};

template<class node_handle_t>
class sorted_list : public abstract_list<node_handle_t> {
   using super_type     = abstract_list<node_handle_t>;
   using value_type     = typename super_type::value_type;
   using key_type       = typename super_type::key_type;    
   using class_type     = typename super_type::class_type;    
   using iterator       = typename super_type::iterator;   
   using const_iterator = typename super_type::const_iterator; 
   using node_handle_type = typename super_type::node_handle_type; 


   using super_type::m_elements;
   using super_type::m_values;
   using super_type::clear;
   public:

   using super_type::end;
   using super_type::begin;

   const_iterator find(const key_type& key) const {
      const size_t position = locate(key);
      if(position < m_elements && node_handle_type::key(m_values[position]) == key) return {*this, position };
      return end();
   }

   virtual size_t locate(const key_type& key) const override {
#ifndef NDEBUG
      size_t ret = m_elements;
      for(size_t position = 0; position < m_elements; ++position) {
         if(node_handle_type::key(m_values[position]) >= key) { ret= position; break; }
      }
      for(size_t i = 1; i< m_elements; ++i) { 
         DCHECK_LT(  node_handle_type::key(m_values[i-1]),  node_handle_type::key(m_values[i])); 
      }
      std::vector<key_type> v;
      for(size_t i = 0; i< m_elements; ++i) { 
         v.push_back(node_handle_type::key(m_values[i]));
      }
#endif// NDEBUG

      size_t left = 0;
      size_t right = m_elements;
      while(left+1 < right) {
         const size_t med = (left+right)/2;
         const key_type medKey = node_handle_type::key(m_values[med]);
         if(medKey == key) { break; }
         if(medKey < key) { 
            left = med+1;
            DCHECK_LE(left, ret);
         } else {
            right=med;
            DCHECK_LE(ret, med);
         }
      }
      const size_t med = (left+right)/2;
#ifndef NDEBUG
      if(ret < m_elements) {
         if(v[ret] == key) DCHECK_EQ(ret, med);
         else { 
            if(med > 0) DCHECK_LT(v[med-1], key);
            if(med+1 < m_elements) DCHECK_LT(key, v[med+1]);
         }
      } 
#endif
      return med;
   }

   void insert(const value_type& value) {
      const key_type key = node_handle_type::key(value);
      size_t position = locate(key);
      if(position < m_elements) {
         const key_type position_key = node_handle_type::key(m_values[position]);
         if(position_key == key) return; // already inserted
         if(position_key < key) ++position;
      }

      ++m_elements;
      m_values = reinterpret_cast<value_type*>  (realloc(m_values, sizeof(value_type)*m_elements));

      for(size_t i = m_elements-1; i > position; --i) {
         m_values[i] = m_values[i-1];
      }

      m_values[position] = value;
#ifndef NDEBUG
      for(size_t i = 1; i< m_elements; ++i) { 
         DCHECK_LT(  node_handle_type::key(m_values[i-1]),  node_handle_type::key(m_values[i])); 
      }
#endif
   }
};



template<class node_handle_t>
class unsorted_list : public abstract_list<node_handle_t> {
   using class_type     = unsorted_list<node_handle_t>;
   using super_type     = abstract_list<node_handle_t>;

   using value_type     = typename super_type::value_type;
   using key_type       = typename super_type::key_type;    
   using iterator       = typename super_type::iterator;   
   using const_iterator = typename super_type::const_iterator; 
   using node_handle_type = typename super_type::node_handle_type; 

   using super_type::m_elements;
   using super_type::m_values;
   using super_type::clear;

   public:
   using super_type::end;
   using super_type::begin;

   const_iterator find(const key_type& key) const {
      const size_t position = locate(key);
      if(position < m_elements) return {*this, position };
      return end();
   }

   size_t locate(const key_type& key) const {
      for(size_t position = 0; position < m_elements; ++position) {
         const key_type pos_key = node_handle_type::key(m_values[position]);
         if(pos_key == key) return position;
      }
      return -1ULL;
   }

   void insert(const value_type& value) {
      const key_type key = node_handle_type::key(value);
      const size_t position = locate(key);
      if(position < m_elements && node_handle_type::key(m_values[position]) == key) return; // already inserted

      ++m_elements;
      m_values = reinterpret_cast<value_type*>  (realloc(m_values, sizeof(value_type)*m_elements));
      m_values[m_elements-1] = value;
   }
};




#if defined(USE_CUCKOOHASH)

template<class map_t>
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
      : m_map(map), m_position(position) 
   { }
   cuckoo_hash_iterator& operator++() {
      do {
         ++m_position;
      } while(m_position < m_map.capacity() && m_map.value(m_position) != m_map.empty_value());
      return *this;
   }
   bool invalid() const {
      return m_position >= m_map.capacity() || m_map.value(m_position) == m_map.empty_value();
   }
   template<class U>
   bool operator!=(cuckoo_hash_iterator<U> o) const { return ! (this->operator==(o)); }

   template<class U>
   bool operator==(cuckoo_hash_iterator<U> o) const {
      if( o.invalid() && invalid()) return true;
      if(!o.invalid() && !invalid()) return m_position == o.m_position;
      return false;
   }
   value_type value() const {
      DCHECK(!invalid());
      return m_map.value(m_position);
   }
   key_type key() const {
      return node_handle_type::key(value());
   }

};


class cuckoo_hash_function {
   private:
	uint64_t seed_a;
	uint64_t seed_b;

   public:
	cuckoo_hash_function() 
       : seed_a(((static_cast<uint64_t>(rand()) << 32) | rand())|1ULL)
       , seed_b(((static_cast<uint64_t>(rand()) << 32) | rand())|1ULL)
   { }

   uint64_t operator()(uint64_t h) const { // based from murmur64
	  h ^= h >> 33;
	  h *= seed_a;
	  h ^= h >> 33;
	  h *= seed_b;
	  h ^= h >> 33;
	  return h;
   }
};

namespace std {
template<class map_t>
std::string to_string(const cuckoo_hash_iterator<map_t>& it)  {
   std::stringstream ss;
   ss << "position: " << it.position() << "; ";
   return ss.str();
}
}

#ifndef NUM_CUCKOO_HASH_FUNCTIONS
#define NUM_CUCKOO_HASH_FUNCTIONS 2
#endif



template<class node_handle_t>
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
#endif// NUM_CUCKOO_HASH_FUNCTIONS == 3

   const value_type m_empty_value;


   iterator begin() {
      if(m_buckets == 0) return end();
      const size_t cbucket_count = capacity();
      for(size_t bucket = 0; bucket < cbucket_count; ++bucket) {
         if(m_values[bucket] != m_empty_value) return { *this, bucket };
      }
      return end();
   }
   const_iterator begin() const {
      if(m_buckets == 0) return end();
      const size_t cbucket_count = capacity();
      for(size_t bucket = 0; bucket < cbucket_count; ++bucket) {
         if(m_values[bucket] != m_empty_value) return { *this, bucket };
      }
      return end();
   }
   iterator end() {
      return {*this, -1ULL };
   }
   const_iterator end() const {
      return {*this, -1ULL };
   }

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

   size_t size() const {
      return m_elements;
   }
   void clear() {
      if(m_values != nullptr) {
         free(m_values);
         m_values = nullptr;
      }
      m_elements = 0;
   }
   ~cuckoo_hash() { clear(); }
   
   cuckoo_hash(const size_t bucket_count, const value_type empty_value) 
      : m_empty_value(empty_value)
   {
      if(bucket_count > 0) { reserve(bucket_count); }
   }

   size_t capacity() const {
      return 1ULL<<m_buckets;
   }

   void reserve(size_t reserve) {
      size_t reserve_bits = Fast::mostSignificantBit(reserve);
      if(1ULL<<reserve_bits != reserve) ++reserve_bits;
      const size_t new_size = 1ULL<<reserve_bits;
        if(m_buckets == 0) {
            m_values = reinterpret_cast<value_type*>  (malloc(new_size*sizeof(value_type)));
            std::fill(m_values, m_values+new_size, m_empty_value);
            m_buckets = reserve_bits;
        } else {
            class_type tmp_map(new_size, m_empty_value);
#if STATS_ENABLED
            tdc::StatPhase statphase(std::string("resizing to ") + std::to_string(reserve_bits));
            print_stats(statphase);
#endif
            const size_t cbucket_count = capacity();
            for(size_t bucket = 0; bucket < cbucket_count; ++bucket) {
               const value_type& value = m_values[bucket];
               if(value == m_empty_value) continue;
               tmp_map.insert(value);
            }
            swap(tmp_map);
        }
    }
   static float m_load_factor;

   static float max_load_factor() {
      return m_load_factor;
   }
   static void max_load_factor(float ml) {
      m_load_factor = ml;
   }

   const_iterator find(const key_type& key) const {
      if(m_buckets == 0) return end();

      {// a
         const size_t position_a = m_hash_a(key) & (-1ULL >> (64-m_buckets) );
         if(m_values[position_a] != m_empty_value) {
            const key_type position_a_key = node_handle_type::key(m_values[position_a]);
            if(position_a_key == key) {
               return const_iterator { *this, position_a };
            }
         }
      }
      {// b
         const size_t position_b = m_hash_b(key) & (-1ULL >> (64-m_buckets) );
         if(m_values[position_b] != m_empty_value) {
            const key_type position_b_key = node_handle_type::key(m_values[position_b]);
            if(position_b_key == key) {
               return const_iterator { *this, position_b };
            }
         }
      }
#if NUM_CUCKOO_HASH_FUNCTIONS == 3
      {// c
         const size_t position_c = m_hash_c(key) & (-1ULL >> (64-m_buckets) );
         if(m_values[position_c] != m_empty_value) {
            const key_type position_c_key = node_handle_type::key(m_values[position_c]);
            if(position_c_key == key) {
               return const_iterator { *this, position_c };
            }
         }
      }
#endif//USE_THIRD_HASH_FUNCTION

      return end();
   }

   bool erase(const key_type& key) {
      if(m_buckets == 0) return false;
      {// a
         const size_t position_a = m_hash_a(key) & (-1ULL >> (64-m_buckets) );
         if(m_values[position_a] != m_empty_value) {
            const key_type position_a_key = node_handle_type::key(m_values[position_a]);
            if(position_a_key == key) {
               m_values[position_a] = m_empty_value;
               --m_elements;
               return true;
            }
         }
      }
      {// b
         const size_t position_b = m_hash_b(key) & (-1ULL >> (64-m_buckets) );
         if(m_values[position_b] != m_empty_value) {
            const key_type position_b_key = node_handle_type::key(m_values[position_b]);
            if(position_b_key == key) {
               m_values[position_b] = m_empty_value;
               --m_elements;
               return true;
            }
         }
      }
#if NUM_CUCKOO_HASH_FUNCTIONS == 3
      {// c
         const size_t position_c = m_hash_c(key) & (-1ULL >> (64-m_buckets) );
         if(m_values[position_c] != m_empty_value) {
            const key_type position_c_key = node_handle_type::key(m_values[position_c]);
            if(position_c_key == key) {
               m_values[position_c] = m_empty_value;
               --m_elements;
               return true;
            }
         }
      }
#endif//USE_THIRD_HASH_FUNCTION

      return false;
   }

   void insert(const value_type& value) {
      insert(value, 0);
   }

   private:
   constexpr static size_t MAX_TRIES = 1000; // number cuckoo replacement steps before considering this as an endless loop -> resort to rehashing
   void insert(const value_type& value, const size_t tries) {
      const key_type key = node_handle_type::key(value);
      if(m_elements+1  > capacity() * max_load_factor() || tries > MAX_TRIES) {
         reserve(capacity()<<1);
      }

      const size_t position_a = m_hash_a(key) & (-1ULL >> (64-m_buckets) );
      if(m_values[position_a] == m_empty_value) {
         m_values[position_a] = value;
         ++m_elements;
         return;
      }
      const key_type position_a_key = node_handle_type::key(m_values[position_a]);
      if(position_a_key == key) {
         m_values[position_a] = value;
         return;
      }
      const size_t position_b = m_hash_b(key) & (-1ULL >> (64-m_buckets) );
      if(m_values[position_b] == m_empty_value) {
         m_values[position_b] = value;
         ++m_elements;
         return;
      }
      const key_type position_b_key = node_handle_type::key(m_values[position_a]);
      if(position_b_key == key) {
         m_values[position_b] = value;
         return;
      }
#if NUM_CUCKOO_HASH_FUNCTIONS == 3
      const size_t position_c = m_hash_c(key) & (-1ULL >> (64-m_buckets) );
      if(m_values[position_c] == m_empty_value) {
         m_values[position_c] = value;
         ++m_elements;
         return;
      }
      const key_type position_c_key = node_handle_type::key(m_values[position_a]);
      if(position_c_key == key) {
         m_values[position_c] = value;
         return;
      }
#endif

      switch(rand() % NUM_CUCKOO_HASH_FUNCTIONS) {
         case 1:
            {
               const value_type tmp_value = m_values[position_a];
               m_values[position_a] = value;
               insert(tmp_value, tries+1);
               return;
            }
#if NUM_CUCKOO_HASH_FUNCTIONS == 3
         case 2:
            {
               const value_type tmp_value = m_values[position_c];
               m_values[position_c] = value;
               insert(tmp_value, tries+1);
               return;
            }
#endif
         default:
            {
               const value_type tmp_value = m_values[position_b];
               m_values[position_b] = value;
               insert(tmp_value, tries+1);
               return;
            }
      }
   }


};

template<class node_handle_t> float cuckoo_hash<node_handle_t>::m_load_factor = 0.8f;
template<class node_handle_t> cuckoo_hash_function cuckoo_hash<node_handle_t>::m_hash_a;
template<class node_handle_t> cuckoo_hash_function cuckoo_hash<node_handle_t>::m_hash_b;
#if NUM_CUCKOO_HASH_FUNCTIONS == 3
template<class node_handle_t> cuckoo_hash_function cuckoo_hash<node_handle_t>::m_hash_c;
#endif// NUM_CUCKOO_HASH_FUNCTIONS == 3

#endif //defined(USE_CUCKOOHASH)


template <typename Value>
class TFastTrie {


   public:
    class Node {



       public:
        typedef typename Factory<Node>::Index Index;

       struct child_handler {
          using key_type = Uchar;
          using node_type = Node;
          using index_type = Index;
          static key_type key(const index_type& index) {
             return TFastTrie::m_node_factory->at(index).key();
          }
       };

       struct long_node_handler {
          using key_type = Ulong;
          using node_type = Node;
          using index_type = Index;
          static key_type key(const index_type& index) {
             return TFastTrie::m_node_factory->at(index).handle();
          }
       };



#ifndef NDEBUG
        typedef rigtorp::HashMap<Uchar, Index, Fast::UcharHash, Fast::UcharEqual> Char2NodeMap;
#endif
        // typedef std::unordered_map<Uchar, Node *> Char2NodeMap;
        // typedef std::map<Uchar, Node *> Char2NodeMap;

        Value value_;
        Ulong extent_;
        Uchar nameLength_;
        ON_DEBUG(Char2NodeMap children_;)
        //unsorted_list<child_handler> m_children;
        sorted_list<child_handler> m_children;
        //cuckoo_hash<Node> m_children;

        inline Node()
            : value_(0),
              extent_(0),
              nameLength_(0)
       //, m_children(1,static_cast<Index>(NodeFactory::INDEX_NULL)) 
       {
       
#ifndef NDEBUG
              children_ = Char2NodeMap(1, 0);
#endif
       }

        inline void set(const Value &value, const Ulong &extent) {
            value_ = value;
            extent_ = extent;
            nameLength_ = 1;
        }

        inline void set(const Value &value, const Ulong &extent,
                        const Int &nameLength) {
            value_ = value;
            extent_ = extent;
            nameLength_ = nameLength;
        }

        inline Ulong handleLength() const {
            return Fast::twoFattest(nameLength_ - 1, extentLength());
        }

        inline Ulong handle() const {
            return extent_ <= 0
                       ? 0
                       : LongString::toSubLong(extent_, 0, handleLength());
        }

        inline static Ulong handle(Ulong extent, Ulong nameLength) {
            return extent <= 0
                       ? 0
                       : LongString::toSubLong(
                             extent, 0,
                             Fast::twoFattest(nameLength - 1,
                                              LongString::charSize(extent)));
        }

        inline Ulong extentLength() const {
            return LongString::charSize(extent_);
        }
        inline Uchar key() const {
            return LongString::toChar(extent_, nameLength_ - 1);
        }
        constexpr static Factory<Node>& factory() {
            return *TFastTrie::m_node_factory;
        }

        inline bool isLeaf() const { 
           DCHECK_EQ(children_.size(), m_children.size());
           return m_children.size() == 0; }

        inline void printChiledren() const {
            // INFO("print TFastTrie Node Chiledren");
            // auto iter = m_children.begin();
            // INFO("\t" << LongString::toString(extent_) << "\"");
            // for (; iter != m_children.end(); ++iter) {
            //     INFO("\t\t"
            //          << "\"" << iter.key() << "\" -> \""
            //          << LongString::toString(iter->value->extent_) << "\"");
            // }
        }
    };

   public:
    typedef Factory<Node> NodeFactory;

   static NodeFactory *m_node_factory;

   private:
    typedef typename NodeFactory::Index NodeIndex;
#ifndef NDEBUG
    typedef rigtorp::HashMap<Ulong, NodeIndex, Fast::Hash, Fast::Equal> Handle2NodeMap;
#endif

    Int size_;
    NodeIndex root_;
    ON_DEBUG(Handle2NodeMap handle2NodeMap_;)


#ifdef USE_NEWDATASTRUCTURE
#ifdef USE_CUCKOOHASH
    using handle2node_map_type = cuckoo_hash<typename Node::long_node_handler>;
#else
    using handle2node_map_type = sorted_list<typename Node::long_node_handler>;
#endif//USE_CUCKOOHASH
    // using handle2node_map_type = unsorted_list<typename Node::long_node_handler>;
    handle2node_map_type m_handle2node_map;
#else
    using handle2node_map_type = rigtorp::HashMap<Ulong, NodeIndex, Fast::Hash, Fast::Equal>;
    handle2node_map_type m_handle2node_map;
#endif

   public:
    inline TFastTrie()
        : size_(0)
        , root_(NodeFactory::INDEX_NULL)
#if defined(USE_NEWDATASTRUCTURE) && defined(USE_CUCKOOHASH)
        , m_handle2node_map(0, static_cast<typename handle2node_map_type::value_type>(NodeFactory::INDEX_NULL)) // cuckoo hash constructor
#endif
   {
          ON_DEBUG(handle2NodeMap_ = Handle2NodeMap(1, NodeFactory::INDEX_NULL);)
#ifndef USE_NEWDATASTRUCTURE
          m_handle2node_map = handle2node_map_type(1, NodeFactory::INDEX_NULL);
#endif
   }

    inline void set(NodeFactory *nodeFactory) { 
       if(m_node_factory == nullptr) m_node_factory = nodeFactory;
       else DCHECK_EQ(reinterpret_cast<size_t>(m_node_factory), reinterpret_cast<size_t>(nodeFactory));
    }

    inline void add(const Ulong &newText, const Value &value) {
        // LOG("add new text into TFastTrie : " << newText);
        // assert(!contains(newText));

        if (size_ == 0) {
            root_ = makeNodeIndex();
            node(root_).set(value, newText);
        } else {
            NodeIndex exitNodeIndex = getExitNodeIndex(newText);
            Node &exitNode = node(exitNodeIndex);
            assert(exitNodeIndex != NodeFactory::INDEX_NULL);
            Int lcpLength =
                LongString::getCharLCPLength(exitNode.extent_, newText);
            if (lcpLength == LongString::charSize(newText)) {
                LOG("new text already exist.");
                return;
            }

            if (lcpLength < exitNode.extentLength()) {
                Ulong exitNode_extent = exitNode.extent_;

                Ulong newExtent =
                    (lcpLength == 0)
                        ? 0
                        : LongString::toSubLong(exitNode.extent_, 0, lcpLength);
                if (exitNode.isLeaf()) {
                    exitNode.extent_ = newExtent;
                    addNode(exitNodeIndex);
                } else {
                    Ulong newHandle =
                        Node::handle(newExtent, exitNode.nameLength_);
                    bool isChangeHandle = (exitNode.handle() != newHandle);
                    if (isChangeHandle) {
                        removeNode(exitNode.handle());
                        exitNode.extent_ = newExtent;
                        addNode(exitNodeIndex);
                    } else {
                        exitNode.extent_ = newExtent;
                    }
                }

                NodeIndex newNodeIndex = makeNodeIndex();
                Node &newNode = node(newNodeIndex);
                newNode.set(exitNode.value_, exitNode_extent, lcpLength + 1);

                exitNode.m_children.swap(newNode.m_children);
                exitNode.m_children.insert(newNodeIndex);

                ON_DEBUG(exitNode.children_.swap(newNode.children_);)
                ON_DEBUG(exitNode.children_[LongString::toChar(exitNode_extent, lcpLength)] = newNodeIndex;)

                DCHECK_EQ(exitNode.m_children.size(), exitNode.children_.size());

                if (!newNode.isLeaf()) {
                    addNode(newNodeIndex);
                }
            } else {
                if (exitNode.isLeaf()) {
                    addNode(exitNodeIndex);
                }
            }
            NodeIndex newTextNodeIndex = makeNodeIndex();
            node(newTextNodeIndex).set(value, newText, lcpLength + 1);
            // NodeIndex newTextNode = new Node(value, newText, lcpLength + 1);
            // addNode(newTextNode);
            exitNode.m_children.insert(newTextNodeIndex);
            ON_DEBUG(exitNode.children_[LongString::toChar(newText, lcpLength)] = newTextNodeIndex;)
            DCHECK_EQ(exitNode.m_children.size(), exitNode.children_.size());
        }

        ++size_;
        assert(hasPrefix(newText));
    }

    inline void addNode(const NodeIndex &nodeIndex) {
        assert(handle2NodeMap_.count(node(nodeIndex).handle()) == 0);
        ON_DEBUG(handle2NodeMap_[node(nodeIndex).handle()] = nodeIndex;)
#ifdef USE_NEWDATASTRUCTURE
        DCHECK_EQ(m_handle2node_map.find(nodeIndex), m_handle2node_map.end());
        m_handle2node_map.insert(nodeIndex);
        DCHECK_EQ(m_handle2node_map.size(), handle2NodeMap_.size());
        ON_DEBUG(bisimulate();)
#else
        m_handle2node_map[node(nodeIndex).handle()] = nodeIndex;
#endif
    }
   

#ifndef NDEBUG
#ifdef USE_NEWDATASTRUCTURE
    void bisimulate() {
       for(auto it = handle2NodeMap_.begin(); it != handle2NodeMap_.end(); ++it) {
          DCHECK_EQ( it->first, node(it->second).handle());
       }
       for(auto it = handle2NodeMap_.begin(); it != handle2NodeMap_.end(); ++it) {
          auto it2 = m_handle2node_map.find(it->first);
          DCHECK(it2 != m_handle2node_map.end());
          DCHECK_EQ(it->second, it2.value());
       }
       for(auto it = m_handle2node_map.begin(); it != m_handle2node_map.end(); ++it) {
          auto it2 = handle2NodeMap_.find(it.key());
          DCHECK(it2 != handle2NodeMap_.end());
          DCHECK_EQ(it2->second, it.value());
       }

    }
#endif// USE_NEWDATASTRUCTURE
#endif//NDEBUG

    // inline void removeNode(NodeIndex   node) {
    // removeNode(node->handle()); }

    inline void removeNode(const Ulong &handle) {
#ifdef USE_NEWDATASTRUCTURE
        ON_DEBUG(bisimulate();)
       DCHECK_NE(m_handle2node_map.find(handle), m_handle2node_map.end());
#endif// USE_NEWDATASTRUCTURE
        assert(handle2NodeMap_.count(handle) != 0);
        ON_DEBUG(handle2NodeMap_.erase(handle);)
      m_handle2node_map.erase(handle);
        DCHECK_EQ(m_handle2node_map.size(), handle2NodeMap_.size());
#ifdef USE_NEWDATASTRUCTURE
        ON_DEBUG(bisimulate();)
#endif// USE_NEWDATASTRUCTURE
    }

    inline bool hasPrefix(const Ulong &pattern) const {
        NodeIndex nodeIndex = getExitNodeIndex(pattern);
        return nodeIndex == NodeFactory::INDEX_NULL
                   ? false
                   : LongString::isCharPrefix(node(nodeIndex).extent_, pattern);
    }

    inline NodeIndex getExitNodeIndex(const Ulong &pattern) const {
        LOG("getExitNode of " << LongString::toString(pattern));

        Int pattern_length = LongString::charSize(pattern);
        Int a = 0;
        Int b = pattern_length;
        Int f;
        NodeIndex nodeIndex = NodeFactory::INDEX_NULL;
        NodeIndex exitNodeIndex = root_;

        while (0 < b - a) {
            f = Fast::twoFattest(a, b);
            nodeIndex = getNodeIndex(LongString::toSubLong(pattern, 0, f));
            if (nodeIndex != NodeFactory::INDEX_NULL) {
                a = node(nodeIndex).extentLength();
                assert(f <= a);
                if (f > a) {
                    // TRACE(node(nodeIndex).text_.toString());
                    exit(0);
                }
                exitNodeIndex = nodeIndex;
            } else {
                b = f - 1;
            }
        }

        if (exitNodeIndex != NodeFactory::INDEX_NULL) {
            const Node &exitNode = node(exitNodeIndex);
            Int lcpLength =
                LongString::getCharLCPLength(exitNode.extent_, pattern);
            if (lcpLength == exitNode.extentLength() &&
                lcpLength < pattern_length) {
               // ON_DEBUG(size_t extent = LongString::toChar(pattern, lcpLength); )
               //    DCHECK_EQ(extent, exitNode.key());
               //
                ON_DEBUG(const size_t key = LongString::toChar(pattern, lcpLength);)

                auto iter2 = exitNode.m_children.find(LongString::toChar(pattern, lcpLength));
                if (iter2 != exitNode.m_children.end()) {
                    assert(LongString::toChar(pattern, lcpLength) == iter2.key());
                    exitNodeIndex = iter2.value();
                }

#ifndef NDEBUG
                auto iter = exitNode.children_.find(LongString::toChar(pattern, lcpLength));
                if (iter != exitNode.children_.end()) {
                   DCHECK(iter2 != exitNode.m_children.end());
                    assert(LongString::toChar(pattern, lcpLength) == node(iter->second).key());
                    DCHECK_EQ(exitNodeIndex, iter->second);
                } else DCHECK(iter2 == exitNode.m_children.end());
#endif//NDEBUG

            }
        }

        return exitNodeIndex;
    }

    inline NodeIndex getNodeIndex(const Ulong &handle) const {
        auto iter2 = m_handle2node_map.find(handle);
#ifndef USE_NEWDATASTRUCTURE
        NodeIndex ret2 = iter2 == m_handle2node_map.end() ? NodeFactory::INDEX_NULL : iter2->second;
#else
        NodeIndex ret2 = iter2 == m_handle2node_map.end() ? NodeFactory::INDEX_NULL : iter2.value();
#endif

#ifndef NDEBUG
        auto iter = handle2NodeMap_.find(handle);
        NodeIndex ret = iter == handle2NodeMap_.end() ? NodeFactory::INDEX_NULL : iter->second;
        DCHECK_EQ(ret2, ret);
#endif//NDEBUG
        return ret2;
    }

    inline NodeIndex makeNodeIndex() { return m_node_factory->make(); }

    inline Node &node(const NodeIndex &index) {
        return m_node_factory->at(index);
    }

    inline const Node &node(const NodeIndex &index) const {
        return m_node_factory->at(index);
    }

    inline void print() const {
        INFO("print TFastTrie");
        auto iter = m_handle2node_map.begin();
        for (; iter != m_handle2node_map.end(); ++iter) {
            INFO("    \"" << LongString::toString(iter.key()) << "\"\t-> \""
                          << LongString::toString(m_node_factory->at(iter->value()).extent_)
                          << "\"\t(" << iter->key() << "\t-> "
                          << m_node_factory->at(iter->value()).extent_ << ")");
        }
    }
};

template <typename Value> typename TFastTrie<Value>::NodeFactory* TFastTrie<Value>::m_node_factory = nullptr;

