#ifndef T_FAST_TRIE_HPP
#define T_FAST_TRIE_HPP

#include "CuckooHash.hpp"
#include "LongStringPC.hpp"
#include "dcheck.hpp"

#ifdef NDEBUG
#define ON_DEBUG(x)
#else
#define ON_DEBUG(x) x
#endif

#define USE_NEWDATASTRUCTURE
#define USE_CUCKOOHASH

namespace ctriepp {

template <typename Value, const Value EMPTY_VALUE = -1>
class AlphabetAwareZFastTrie {
   public:
    class Node {
       public:
        typedef Factory<Node> NodeFactory;
        typedef typename NodeFactory::Index Index;
        static NodeFactory nodeFactory_;
        constexpr static Index INDEX_NULL = NodeFactory::INDEX_NULL;

        struct child_handler {
            using key_type = Uchar;
            using node_type = Node;
            using index_type = Index;
            static key_type key(const index_type &index) {
                return at(index).key();
            }
        };

        struct long_node_handler {
            using key_type = Ulong;
            using node_type = Node;
            using index_type = Index;
            static key_type key(const index_type &index) {
                return at(index).handle();
            }
        };

        Value value_;
        Ulong extent_;
        Uchar nameLength_;
        // typedef Char2NodeMap = std::unordered_map<Uchar, Node *>;
        // typedef Char2NodeMap = std::map<Uchar, Node *>;
        using Char2NodeMap = sorted_list<child_handler>;
        Char2NodeMap children_;

        inline Node() : value_(EMPTY_VALUE), extent_(0), nameLength_(0) {}

        inline static Index make() { return nodeFactory_.make(); }

        inline static Node &at(const Index &index) {
            return nodeFactory_.at(index);
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

        inline bool isLeaf() const { return children_.size() == 0; }

        inline void print(const std::string indent = "") const {
            INFO(indent << "AlphabetAwareZFastTrie Node \""
                        << LongString::toString(extent_) << "\" : " << value_);
            auto iter = children_.begin();
            if (0 < children_.size()) {
                INFO(indent << "children " << children_.size());
                for (; iter != children_.end(); ++iter) {
                    Node::at(iter.value()).print(indent + "    ");
                }
            }
        }

        // STORE AND LOAD NOT YET FINISHED
        inline Ulong store(std::ofstream& out) const
        {
            std::string indent = "  ";
            INFO(indent << "AlphabetAwareZFastTrie Node \""
                        << LongString::toString(extent_) << "\" : " << value_);
            auto iter = children_.begin();
            if (0 < children_.size()) {
                INFO(indent << "children " << children_.size());
                for (; iter != children_.end(); ++iter) {
                    Node::at(iter.value()).print(indent + "    ");
                }
            }

            Ulong no_bytes = 0;

            out.write((char*)&value_,sizeof(Value));
            out.write((char*)&extent_,sizeof(Ulong));
            out.write((char*)&nameLength_,sizeof(Uchar));
            no_bytes += sizeof(Value) + sizeof(Ulong) + sizeof(Uchar);

            Ulong no_children = children_.size();
            out.write((char*)&no_children,sizeof(Ulong));
            no_bytes += sizeof(Ulong) + children_.store(out);

            return no_bytes;
        }
        inline void load(std::ifstream& in)
        {
            in.read(reinterpret_cast<char*>(&value_),sizeof(Value));
            in.read(reinterpret_cast<char*>(&extent_),sizeof(Ulong));
            in.read(reinterpret_cast<char*>(&nameLength_),sizeof(Uchar));
            Ulong no_children;
            in.read(reinterpret_cast<char*>(&no_children),sizeof(Ulong));

            if (0 < no_children) { children_.load(in); }
        }

        inline Ulong getLeftmostLeaf() const
        {
            if(children_.size()==0)
                return value_;

            auto iter = children_.begin();
            return Node::at(iter.value()).getLeftmostLeaf();
        }
    };

    typedef Factory<Node> NodeFactory;
    typedef typename NodeFactory::Index NodeIndex;

    Int size_;
    NodeIndex root_;

   public:
    #ifdef USE_NEWDATASTRUCTURE
    #ifdef USE_CUCKOOHASH
    using Handle2NodeMap = cuckoo_hash<typename Node::long_node_handler>;
    #else
    using Handle2NodeMap = sorted_list<typename Node::long_node_handler>;
    #endif  // USE_CUCKOOHASH
    // using Handle2NodeMap = unsorted_list<typename Node::long_node_handler>;
    Handle2NodeMap handle2NodeMap_;
    #else
    using Handle2NodeMap =
        rigtorp::HashMap<Ulong, NodeIndex, Fast::Hash, Fast::Equal>;
    Handle2NodeMap handle2NodeMap_;
    #endif

   public:
    inline AlphabetAwareZFastTrie()
        : size_(0),
          root_(Node::INDEX_NULL)
        #if defined(USE_NEWDATASTRUCTURE) && defined(USE_CUCKOOHASH)
          ,
          handle2NodeMap_(0,
                          static_cast<typename Handle2NodeMap::value_type>(
                              Node::INDEX_NULL))  // cuckoo hash constructor
        #endif
    {
        #ifndef USE_NEWDATASTRUCTURE
        handle2NodeMap_ = Handle2NodeMap(1, Node::INDEX_NULL);
        #endif
    }

    typedef Factory<AlphabetAwareZFastTrie> TrieFactory;
    typedef typename TrieFactory::Index Index;
    static TrieFactory trieFactory_;
    constexpr static Index INDEX_NULL = TrieFactory::INDEX_NULL;

    inline static Index make() { return trieFactory_.make(); }

    inline static AlphabetAwareZFastTrie &at(const Index &index) {
        return trieFactory_.at(index);
    }

    inline void insert(const Ulong &newText, const Value &value) {
        LOG("insert new text into AlphabetAwareZFastTrie : "
            << LongString::toString(newText));
        // assert(!contains(newText));
        assert(value != EMPTY_VALUE);

        if (size_ == 0) {
            root_ = Node::make();
            Node::at(root_).set(value, newText);
        } else {
            NodeIndex exitNodeIndex = getExitNodeIndex(newText);
            Node &exitNode = Node::at(exitNodeIndex);
            assert(exitNodeIndex != Node::INDEX_NULL);
            Int lcpLength =
                LongString::getCharLCPLength(exitNode.extent_, newText);

            if (lcpLength < exitNode.extentLength()) {
                Ulong exitNode_extent = exitNode.extent_;

                Ulong newExtent =
                    (lcpLength == 0)
                        ? 0
                        : LongString::toSubLong(exitNode.extent_, 0, lcpLength);
                if (exitNode.isLeaf()) {
                    exitNode.extent_ = newExtent;
                    insertHandle2NodeMap(exitNodeIndex);
                } else {
                    Ulong newHandle =
                        Node::handle(newExtent, exitNode.nameLength_);
                    bool isChangeHandle = (exitNode.handle() != newHandle);
                    if (isChangeHandle) {
                        eraseHandle2NodeMap(exitNode.handle());
                        exitNode.extent_ = newExtent;
                        insertHandle2NodeMap(exitNodeIndex);
                    } else {
                        exitNode.extent_ = newExtent;
                    }
                }

                // make new internal node
                // swap new internal node and exitNode
                NodeIndex newNodeIndex = Node::make();
                Node &newNode = Node::at(newNodeIndex);
                newNode.set(exitNode.value_, exitNode_extent, lcpLength + 1);

                exitNode.children_.swap(newNode.children_);
                exitNode.children_.insert(newNodeIndex);

                if (!newNode.isLeaf()) {
                    insertHandle2NodeMap(newNodeIndex);
                }

                if (lcpLength < LongString::charSize(newText)) {
                    // make new leaf node
                    NodeIndex newTextNodeIndex = Node::make();
                    Node::at(newTextNodeIndex)
                        .set(value, newText, lcpLength + 1);
                    exitNode.value_ = EMPTY_VALUE;
                    exitNode.children_.insert(newTextNodeIndex);
                    Node::at(newTextNodeIndex).value_ = value;
                } else {
                    exitNode.value_ = value;
                }
            } else {
                if (lcpLength == LongString::charSize(newText)) {
                    if (exitNode.value_ == EMPTY_VALUE) {
                        exitNode.value_ = value;
                    } else {
                        WARN("new text already exist.");
                    }
                } else {
                    // if (exitNode.isLeaf()) {
                    if (exitNode.isLeaf()) {
                        insertHandle2NodeMap(exitNodeIndex);
                    }
                    NodeIndex newTextNodeIndex = Node::make();
                    Node::at(newTextNodeIndex)
                        .set(value, newText, lcpLength + 1);
                    exitNode.children_.insert(newTextNodeIndex);
                    Node::at(newTextNodeIndex).value_ = value;
                    // }
                }
            }
        }

        ++size_;
        assert(handle2NodeMap_.size() < size_);
        // TRACE(size_);
        // assert(nodeFactory_.size() < size_ * 2);
        assert(contains(newText));
        assert(containsPrefix(newText));
    }

    inline void erase(const Ulong &targetText) {
        LOG("AlphabetAwareZFastTrie::erase "
            << LongString::toString(targetText));
        // print();
        assert(contains(targetText));
        NodeIndex targetNodeIndex = getExitNodeIndex(targetText);
        assert(targetNodeIndex != Node::INDEX_NULL);
        Node &targetNode = Node::at(targetNodeIndex);
        if (size_ == 0) {
            root_ = Node::INDEX_NULL;
        } else if (size_ <= 1) {
            root_ = Node::INDEX_NULL;
            eraseHandle2NodeMap(targetNode.handle());
        } else {
            Int lcpLength =
                LongString::getCharLCPLength(targetNode.extent_, targetText);
            assert(lcpLength == LongString::charSize(targetText));
            assert(LongString::charSize(targetText) <=
                   targetNode.extentLength());
            if (targetNode.children_.size() == 0) {
                if (targetNodeIndex == root_) {
                    eraseHandle2NodeMap(targetNode.handle());
                } else {
                    NodeIndex parentNodeIndex;
                    if (targetNode.nameLength_ <= 1) {
                        parentNodeIndex = root_;
                    } else {
                        parentNodeIndex = getExitNodeIndex(
                            LongString::toSubLong(targetNode.extent_, 0,
                                                  targetNode.nameLength_ - 1));
                    }

                    assert(parentNodeIndex != Node::INDEX_NULL);
                    Node &parentNode = Node::at(parentNodeIndex);
                    parentNode.children_.erase(targetNode.key());
                    eraseHandle2NodeMap(targetNode.handle());

                    if (parentNode.children_.size() == 1 &&
                        parentNode.value_ == EMPTY_VALUE) {
                        LOG("swap parent and child node");
                        // swap parent and child node
                        eraseHandle2NodeMap(parentNode.handle());
                        Node &childNode =
                            Node::at(parentNode.children_.begin().value());
                        parentNode.set(childNode.value_, childNode.extent_,
                                       parentNode.nameLength_);
                        parentNode.children_.swap(childNode.children_);

                        if (!parentNode.isLeaf()) {
                            eraseHandle2NodeMap(childNode.handle());
                            insertHandle2NodeMap(parentNodeIndex);
                        }
                        // delete parentNode
                    }
                }
            } else if (targetNode.children_.size() == 1) {
                // delete internal node
                Node &childNode =
                    Node::at(targetNode.children_.begin().value());
                eraseHandle2NodeMap(targetNode.handle());
                targetNode.set(childNode.value_, childNode.extent_,
                               targetNode.nameLength_);
                targetNode.children_.swap(childNode.children_);
                if (!targetNode.isLeaf()) {
                    eraseHandle2NodeMap(childNode.handle());
                    insertHandle2NodeMap(targetNodeIndex);
                }
            } else {
                targetNode.value_ = EMPTY_VALUE;
            }
        }
        --size_;
        // if (contains(targetText)) {
        // print();
        // }
        assert(!contains(targetText));
    }

    inline void update(const Ulong &targetText, Value value) {
        NodeIndex exitNodeIndex = getExitNodeIndex(targetText);
        if (exitNodeIndex == Node::INDEX_NULL) {
            WARN("AlphabetAwareZFastTrie::update");
            WARN("Not Found key");
        } else {
            Value &exitNodeValue = Node::at(exitNodeIndex).value_;
            if (exitNodeValue == EMPTY_VALUE) {
                WARN("AlphabetAwareZFastTrie::update");
                WARN("Not Found key");
            } else {
                exitNodeValue = value;
            }
        }
    }

    inline void insertHandle2NodeMap(const NodeIndex &nodeIndex) {
#ifdef USE_NEWDATASTRUCTURE
        handle2NodeMap_.insert(nodeIndex);
#else
        handle2NodeMap_[Node::at(nodeIndex).handle()] = nodeIndex;
#endif
    }

    inline void eraseHandle2NodeMap(const Ulong &handle) {
        // assert(handle2NodeMap_.count(hangit dle) != 0);
        handle2NodeMap_.erase(handle);
        // assert(handle2NodeMap_.count(handle) == 0);
    }

    inline bool contains(const Ulong &pattern) const {
        NodeIndex exitNodeIndex = getExitNodeIndex(pattern);
        if (exitNodeIndex == Node::INDEX_NULL) {
            return false;
        } else {
            Node &exitNode = Node::at(exitNodeIndex);
            return (exitNode.extent_ == pattern) &&
                   (exitNode.value_ != EMPTY_VALUE);
        }
    }

    inline bool containsPrefix(const Ulong &pattern) const {
        NodeIndex nodeIndex = getExitNodeIndex(pattern);
        return nodeIndex == Node::INDEX_NULL
                   ? false
                   : LongString::isCharPrefix(Node::at(nodeIndex).extent_,
                                              pattern);
    }

    inline Ulong getPrefix(const Ulong &pattern) const
    {
        NodeIndex nodeIndex = getExitNodeIndex(pattern);
        //std::cout << "ExitNodeIndex: " << nodeIndex << " - " << Node::at(nodeIndex).extent_ << std::endl;
        //std::cout << "Node::INDEX_NULL: " << Node::INDEX_NULL << std::endl;
        bool matched = (nodeIndex == Node::INDEX_NULL
                        ? false
                        : LongString::isCharPrefix(Node::at(nodeIndex).extent_,
                                                   pattern));
        //std::cout << "matched: " << int(matched) << std::endl;
        //exit(1);
        return matched == false
            ? -1
            : Node::at(nodeIndex).getLeftmostLeaf();
    }

    inline std::pair<Ulong,Int> getLongestPrefix(const Ulong &pattern) const
    {
        NodeIndex nodeIndex = getExitNodeIndex(pattern);
        //std::cout << "ExitNodeIndex: " << nodeIndex << " - " << Node::at(nodeIndex).extent_ << std::endl;
        //std::cout << "Node::INDEX_NULL: " << Node::INDEX_NULL << std::endl;
        Int matchedLength = (nodeIndex == Node::INDEX_NULL
                            ? -1
                            : LongString::getCharLCPLength(Node::at(nodeIndex).extent_,
                                                           pattern));
        //std::cout << "matched: " << int(matched) << std::endl;
        //exit(1);
        return matchedLength == -1
            ? std::make_pair(Ulong(-1),0)
            : std::make_pair(Node::at(nodeIndex).getLeftmostLeaf(),matchedLength);
    }

    inline NodeIndex getExitNodeIndex(const Ulong &pattern) const {
        LOG("getExitNode of " << LongString::toString(pattern));

        Int pattern_length = LongString::charSize(pattern);
        Int a = 0;
        Int b = pattern_length;
        Int f;
        NodeIndex nodeIndex = Node::INDEX_NULL;
        NodeIndex exitNodeIndex = root_;

        while (0 < b - a) {
            f = Fast::twoFattest(a, b);
            nodeIndex = getNodeIndex(LongString::toSubLong(pattern, 0, f));
            if (nodeIndex != Node::INDEX_NULL) {
                a = Node::at(nodeIndex).extentLength();
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

        if (exitNodeIndex != Node::INDEX_NULL) {
            const Node &exitNode = Node::at(exitNodeIndex);
            Int lcpLength =
                LongString::getCharLCPLength(exitNode.extent_, pattern);
            if (lcpLength == exitNode.extentLength() &&
                lcpLength < pattern_length) {
                // ON_DEBUG(size_t extent = LongString::toChar(pattern,
                // lcpLength); )
                //    DCHECK_EQ(extent, exitNode.key());
                //
                ON_DEBUG(const size_t key =
                             LongString::toChar(pattern, lcpLength);)

                auto iter2 = exitNode.children_.find(
                    LongString::toChar(pattern, lcpLength));
                if (iter2 != exitNode.children_.end()) {
                    assert(LongString::toChar(pattern, lcpLength) ==
                           iter2.key());
                    exitNodeIndex = iter2.value();
                }
            }
        }

        return exitNodeIndex;
    }

    inline NodeIndex getNodeIndex(const Ulong &handle) const {
        auto iter2 = handle2NodeMap_.find(handle);
#ifndef USE_NEWDATASTRUCTURE
        NodeIndex ret2 =
            iter2 == handle2NodeMap_.end() ? Node::INDEX_NULL : iter2->second;
#else
        NodeIndex ret2 =
            iter2 == handle2NodeMap_.end() ? Node::INDEX_NULL : iter2.value();
#endif

        return ret2;
    }

    inline void print(const std::string indent = "") const {
        INFO(indent << "print AlphabetAwareZFastTrie");
        auto iter = handle2NodeMap_.begin();
        for (; iter != handle2NodeMap_.end(); ++iter) {
            INFO(indent << "    \"" << LongString::toString(iter.key())
                        << "\"\t-> \""
                        << LongString::toString(Node::at(iter.value()).extent_)
                        << "\"");
        }
        if (root_ != Node::INDEX_NULL) {
            Node::at(root_).print(indent);
        }
    }

    // STORE AND LOAD NOT YET FINISHED
    inline Ulong store(std::ofstream& out) const {
        const std::string indent = "    ";
        INFO(indent << "print AlphabetAwareZFastTrie");

        Ulong no_bytes = 0;

        out.write((char*)&root_,sizeof(NodeIndex));
        out.write((char*)&size_,sizeof(Int));
        std::cout << "root_ " << root_ << std::endl;
        std::cout << "size_ " << size_ << std::endl;

        no_bytes += sizeof(NodeIndex) + sizeof(Int);
        no_bytes += handle2NodeMap_.store(out);

        auto iter = handle2NodeMap_.begin();
        for (; iter != handle2NodeMap_.end(); ++iter) {
            INFO(indent << "    \"" << LongString::toString(iter.key())
                        << "\"\t-> \""
                        << LongString::toString(Node::at(iter.value()).extent_)
                        << "\"");
        }
        if (root_ != Node::INDEX_NULL) {
            Node::at(root_).store(out);
        }

        return no_bytes;
    }
    inline void load(std::ifstream& in)
    {
        in.read(reinterpret_cast<char*>(&root_),sizeof(NodeIndex));
        in.read(reinterpret_cast<char*>(&size_),sizeof(Int));
        //std::cout << "1.1" << std::endl;
        handle2NodeMap_.load(in);
        //std::cout << "1.2" << std::endl;
        if (root_ != Node::INDEX_NULL) {
            Node::at(root_).load(in);
        }
        //std::cout << "1.3" << std::endl;
    }
};

template <typename Value, const Value EMPTY_VALUE>
typename AlphabetAwareZFastTrie<Value, EMPTY_VALUE>::Node::NodeFactory
    AlphabetAwareZFastTrie<Value, EMPTY_VALUE>::Node::nodeFactory_;

template <typename Value, const Value EMPTY_VALUE>
typename AlphabetAwareZFastTrie<Value, EMPTY_VALUE>::TrieFactory
    AlphabetAwareZFastTrie<Value, EMPTY_VALUE>::trieFactory_;

}  // namespace ctriepp

#endif  // T_FAST_TRIE_HPP
