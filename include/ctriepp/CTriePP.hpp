#ifndef C_TRIE_PP_HPP
#define C_TRIE_PP_HPP

#define USE_NEWDATASTRUCTURE
#define USE_CUCKOOHASH

#include "AlphabetAwareZFastTrie.hpp"
// #include "tsl/robin_map.h"

namespace ctriepp {

template <typename Value, const Value EMPTY_VALUE = -1>
class CTriePP {
   public:
    class Node;
    typedef Factory<Node> NodeFactory;
    typedef typename NodeFactory::Index NodeIndex;
    typedef AlphabetAwareZFastTrie<NodeIndex> MicroTrie;
    typedef Factory<MicroTrie> MicroTrieFactory;
    typedef typename MicroTrieFactory::Index MicroTrieIndex;
    typedef typename MicroTrie::Node MicroTrieNode;

    class Node {
       public:
        typedef Factory<Node> NodeFactory;
        typedef typename NodeFactory::Index Index;
        static NodeFactory nodeFactory_;
        constexpr static Index INDEX_NULL = NodeFactory::INDEX_NULL;

        struct long_node_handler {
            using key_type = Ulong;
            using node_type = Node;
            using index_type = NodeIndex;
            static key_type key(const index_type &index) {
                return at(index).key();
            }
        };
        using Word2NodeMap = cuckoo_hash<typename Node::long_node_handler>;

        Value value_;
        LongString subText_;
        Word2NodeMap *children_;
        MicroTrieIndex microTrieIndex_;

        inline Node()
            : subText_(),
              children_(nullptr),
              microTrieIndex_(MicroTrieFactory::INDEX_NULL) {}

        inline static Index make() { return nodeFactory_.make(); }

        inline static Node &at(const Index &index) {
            return nodeFactory_.at(index);
        }

        inline void set(LongString subLongString, const Value &value) {
            value_ = value;
            subText_ = subLongString;
        }

        inline void reset() {
            value_ = EMPTY_VALUE;
            subText_ = LongString();
            delete children_;
            children_ = nullptr;
            if (microTrieIndex_ != MicroTrie::INDEX_NULL) {
                MicroTrie::at(microTrieIndex_).~MicroTrie();
            }
            microTrieIndex_ = MicroTrieFactory::INDEX_NULL;
        }

        inline bool containsChild(const Ulong &key) const {
            return children_ == nullptr
                       ? false
                       : children_->find(key) == children_->end() ? false
                                                                  : true;
        }

        inline void print(const std::string indent = "") const {
            INFO(indent << "CTriePP Node \"" << subText_.toString() << "\", "
                        << value_);
            if (children_ != nullptr) {
                INFO(indent << "children " << children_->size());
                MicroTrie::at(microTrieIndex_).print("  ");
                auto iter = children_->begin();
                for (; iter != children_->end(); ++iter) {
                    Node::at(iter.value()).print(indent + "    ");
                }
            }
        }

        // STORE AND LOAD NOT YET FINISHED
        inline Ulong store(std::ofstream& out) const
        {
            Ulong no_bytes = 0;
            const std::string indent = "";

            //INFO(indent << "CTriePP Node \"" << subText_.toString() << "\", "
            //            << value_);
            out.write((char*)&value_,sizeof(Value));
            no_bytes += sizeof(Value);

            Ulong subText_size = subText_.text_.size();
            out.write((char*)&subText_size,sizeof(Ulong));
            out.write((char*)subText_.text_.data(),subText_.text_.size());
            no_bytes += subText_.text_.size() + sizeof(Ulong);

            Ulong no_children = 0;
            if (children_ != nullptr)
            {
                INFO(indent << "children " << children_->size());
                no_children = children_->size();
                out.write((char*)&no_children,sizeof(no_children));
                no_bytes += sizeof(no_children);
                /* micro trie */
                MicroTrie::at(microTrieIndex_).store(out);
                /* Store Cuckoo hash table */
                no_bytes += children_->store(out);
                // store all children starting from the leftmost one
                /*
                auto iter = children_->begin();
                for (; iter != children_->end(); ++iter)
                {
                    Node::at(iter.value()).print(indent + "    ");
                    no_bytes += Node::at(iter.value()).store(out);
                } */

            }
            else
            { 
                //INFO(indent << "children 0");
                out.write((char*)&no_children,sizeof(Ulong));
                no_bytes += sizeof(Ulong);
                //std::cout << "no_children " << no_children << std::endl;
            }

            return no_bytes;
        }
        inline void load(std::ifstream& in)
        {
            in.read(reinterpret_cast<char*>(&value_),sizeof(Value));
            Ulong subtext_size;
            in.read(reinterpret_cast<char*>(&subtext_size),sizeof(Ulong));
            subText_.text_.resize(subtext_size);
            in.read(&subText_.text_[0],subtext_size);

            Ulong no_children;
            in.read(reinterpret_cast<char*>(&no_children),sizeof(Ulong));

            if(no_children > 0)
            {
                //std::cout << "1" << std::endl;
                microTrieIndex_ = MicroTrie::make();
                MicroTrie::at(microTrieIndex_).load(in);
                //std::cout << "2" << std::endl;
                children_->load(in);
                //std::cout << "3" << std::endl;
            }

            return;
        }


        inline NodeIndex getChildIndex(const Ulong &key) const {
            if (children_ == nullptr) {
                return Node::INDEX_NULL;
            }
            auto iter = children_->find(key);
            return iter == children_->end() ? Node::INDEX_NULL : iter.value();
        }

        inline Ulong key() const {
            // assert(0 < subText_.size());
            if (0 < subText_.size()) {
                return subText_.getLong(0);
            } else {
                return 0;
            }
        }
    };

    NodeIndex root_;

   public:
    inline CTriePP() {
        root_ = Node::INDEX_NULL;
        root_ = Node::make();
        Node::at(root_).set(LongString(), EMPTY_VALUE);
    }

    inline void insert(const std::string *newText, const Value &value) {
        LOG("insert " << *newText);
        insert(LongString(newText), value);
    }

    inline void insert(const LongString &newText, const Value &value) {
        // LOG("insert new text into CTriePP : " << _newText);
        assert(value != EMPTY_VALUE);
        assert(!contains(newText));
        insert(root_, newText, value);
        assert(contains(newText));
        assert(containsPrefix(newText));
    }

    inline void erase(const std::string &targetText) {
        LOG("CTriePP::erase " << targetText);
        erase(LongString(&targetText));
    }

    inline void erase(const LongString &targetText) {
        LongString targetSubText = targetText;
        assert(contains(targetText));
        NodeIndex targetNodeIndex = root_;
        NodeIndex parentNodeIndex = Node::INDEX_NULL;
        while (true) {
            Node &targetNode = Node::at(targetNodeIndex);
            Int lcpLength = getLCPLength(targetSubText, targetNode.subText_);
            if (targetSubText.size() <= lcpLength) {
                // erase node
                eraseNode(parentNodeIndex, targetNode);
                break;
            }

            targetSubText =
                LongString(targetSubText, lcpLength, targetSubText.size());
            Ulong key = targetSubText.getLong(0);
            if (lcpLength == targetNode.subText_.size()) {
                parentNodeIndex = targetNodeIndex;
                targetNodeIndex = targetNode.getChildIndex(key);
                if (targetNodeIndex == Node::INDEX_NULL) {
                    // Fot found!
                    WARN("Fot found erase target node!");
                    break;
                } else {
                    // erase node at next iter
                    continue;
                }
            } else {
                // Fot found!
                WARN("Fot found erase target node!");
                break;
            }
        }
        assert(!contains(targetText));
    }

    inline bool contains(const std::string &targetText) const {
        LOG("contains " << targetText);
        return contains(LongString(&targetText));
    }

    inline bool contains(const LongString &targetText) const {
        LongString targetSubText = targetText;
        NodeIndex targetNodeIndex = root_;
        while (true) {
            Node &targetNode = Node::at(targetNodeIndex);
            Int lcpLength = getLCPLength(targetSubText, targetNode.subText_);
            if (targetSubText.size() <= lcpLength) {
                if (targetSubText.size() == targetNode.subText_.size()) {
                    assert(containsPrefix(targetText));
                    return (targetNode.value_ != EMPTY_VALUE);
                } else {
                    return false;
                }
            }

            targetSubText =
                LongString(targetSubText, lcpLength, targetSubText.size());
            Ulong key = targetSubText.getLong(0);
            if (lcpLength == targetNode.subText_.size()) {
                targetNodeIndex = targetNode.getChildIndex(key);
                if (targetNodeIndex == Node::INDEX_NULL) {
                    return false;
                } else {
                    continue;
                }
            } else {
                return false;
            }
        }
    }

    inline void insert(NodeIndex targetNodeIndex, LongString newSubText,
                       const Value &value)
    {
        while (true) {
            Node &targetNode = Node::at(targetNodeIndex);
            Int lcpLength = getLCPLength(newSubText, targetNode.subText_);

            NodeIndex nextNodeIndex;
            if (lcpLength == targetNode.subText_.size())
            {
                if (lcpLength == newSubText.size())
                {
                    if (targetNode.value_ == EMPTY_VALUE)
                    {
                        targetNode.value_ = value;
                    }
                    else { std::cerr << "Already exist insert key" << std::endl; WARN("Already exist insert key "); }
                    break;
                }
                newSubText =
                    LongString(newSubText, lcpLength, newSubText.size());
                Ulong key = newSubText.getLong(0);
                targetNodeIndex = targetNode.getChildIndex(key);
                if (targetNodeIndex == Node::INDEX_NULL)
                {
                    NodeIndex newNodeIndex = Node::make();
                    Node::at(newNodeIndex).set(newSubText, value);
                    insertChild(targetNode, key, newNodeIndex);
                    break;
                }
                continue;
            } 
            else
            {
                NodeIndex newNodeIndex = Node::make();
                Node &newNode = Node::at(newNodeIndex);
                newNode.set(LongString(targetNode.subText_, lcpLength,
                                       targetNode.subText_.size()),
                            targetNode.value_);
                swapChildren(targetNode, newNode);
                targetNode.subText_.to(lcpLength);
                insertChild(targetNode, newNode.key(), newNodeIndex);
                if (lcpLength < newSubText.size())
                {
                    newSubText =
                        LongString(newSubText, lcpLength, newSubText.size());
                    Ulong key = newSubText.getLong(0);
                    if (newSubText.size() == 0) {
                        targetNode.value_ = value;
                    } else {
                        targetNode.value_ = EMPTY_VALUE;
                        newNodeIndex = Node::make();
                        Node::at(newNodeIndex).set(newSubText, value);
                        insertChild(targetNode, key, newNodeIndex);
                    }
                }
                else { targetNode.value_ = value; }
                break;
            }
        }
    }

    inline Int getLCPLength(const LongString &a, const LongString &b) const {
        Int minLength = std::min(a.size(), b.size());

        for (Int i = 0; i < minLength; ++i) {
            if (a.getLong(i) != b.getLong(i)) {
                return i;
            }
        }
        return minLength;
    }

    inline Int getLCPLength(const LongString &a, const LongString &b,
                            const Int &from, const Int &size) const {
        Int minLength = std::min(a.size(), (Uint)(size - from));
        for (Int i = 0; i < minLength; ++i) {
            if (a.getLong(i) != b.getLong(i + from)) {
                return i;
            }
        }
        return minLength;
    }

    inline bool containsPrefix(const std::string &pattern) const {
        LOG("containsPrefix " << pattern);
        // LongString *pattern = new ;
        return containsPrefix(LongString(&pattern));
    }

    inline bool containsPrefix(const LongString &pattern) const {
        NodeIndex currentNodeIndex = root_;

        Int pattern_size = pattern.size();
        Int from = 0;
        Int lcpLength;
        Ulong key;
        NodeIndex nextNodeIndex;
        while (true) {
            const Node &currentNode = Node::at(currentNodeIndex);
            lcpLength =
                getLCPLength(currentNode.subText_, pattern, from, pattern_size);
            // LOG("LCP Length : " << lcpLength);
            if (lcpLength == pattern_size - from) {
                return true;
            } else {
                key = pattern.getLong(lcpLength + from);
                if (lcpLength == currentNode.subText_.size()) {
                    nextNodeIndex = currentNode.getChildIndex(key);
                    if (nextNodeIndex != Node::INDEX_NULL) {
                        assert(key == (Node::at(nextNodeIndex).key()));
                        from += lcpLength;
                        currentNodeIndex = nextNodeIndex;
                    } else {
                        return containsKeyPrefix(currentNode, key);
                    }
                } else {
                    return LongString::isCharPrefix(
                        currentNode.subText_.getLong(lcpLength), key);
                }
            }
        }
    }

    inline Ulong locatePrefix(const std::string &pattern) const
    {
        return locatePrefix(LongString(&pattern));
    }

    inline Ulong locatePrefix(const LongString &pattern) const
    {
        //std::cout << "locate pattern: " << pattern.toString() << std::endl;
        NodeIndex currentNodeIndex = root_;

        Int pattern_size = pattern.size();
        Int from = 0;
        Int lcpLength;
        Ulong key;
        NodeIndex nextNodeIndex;
        while (true) {
            const Node &currentNode = Node::at(currentNodeIndex);
            lcpLength =
                getLCPLength(currentNode.subText_, pattern, from, pattern_size);
            // LOG("LCP Length : " << lcpLength);
            //std::cout << "LCP Length : " << lcpLength << std::endl;
            if (lcpLength == pattern_size - from) { return currentNode.value_; }
            else
            {
                key = pattern.getLong(lcpLength + from);
                //std::cout << "key : " << key << std::endl;
                if (lcpLength == currentNode.subText_.size())
                {
                    //std::cout << "PRIMA" << std::endl;
                    nextNodeIndex = currentNode.getChildIndex(key);
                    //std::cout << "nextNodeIndex : " << nextNodeIndex << std::endl;
                    if (nextNodeIndex != Node::INDEX_NULL)
                    {
                        assert(key == (Node::at(nextNodeIndex).key()));
                        from += lcpLength;
                        currentNodeIndex = nextNodeIndex;
                    }
                    else
                    {
                        //std::cout << "PRIMA 2" << std::endl;
                        NodeIndex matchingKey = getKeyPrefix(currentNode, key);
                        //std::cout << "matchingKey: " << matchingKey << " - " << (matchingKey == Node::INDEX_NULL) << std::endl;
                        //std::cout << "Node::at(matchingKey).value_: " << Node::at(matchingKey).value_ << std::endl;
                        return matchingKey == Node::INDEX_NULL
                                ? -1
                                : Node::at(matchingKey).value_;
                    }
                }
                else { std::cerr << "Controllare che sia corretto!" << std::endl; exit(1);
                    return LongString::isCharPrefix(
                                currentNode.subText_.getLong(lcpLength), key) == true
                                ? currentNode.value_
                                : -1; }
            }
        }
    }

    inline std::pair<Ulong,Int> locateLongestPrefix(const std::string &pattern) const
    {
        return locateLongestPrefix(LongString(&pattern));
    }

    inline std::pair<Ulong,Int> locateLongestPrefix(const LongString &pattern) const
    {
        NodeIndex currentNodeIndex = root_;

        Int pattern_size = pattern.size();
        Int from = 0;
        Int lcpLength;
        Ulong key;
        NodeIndex nextNodeIndex;
        while (true) {
            const Node &currentNode = Node::at(currentNodeIndex);
            lcpLength =
                getLCPLength(currentNode.subText_, pattern, from, pattern_size);
            // LOG("LCP Length : " << lcpLength);
            //std::cout << "LCP Length : " << lcpLength << std::endl;
            //std::cout << "from : " << from << std::endl;
            //std::cout << "currentNode.subText_ : " << currentNode.subText_.text_ << std::endl;
            if (lcpLength == pattern_size - from) { 
                return std::make_pair(currentNode.value_, lcpLength); }
            else
            {
                key = pattern.getLong(lcpLength + from);
                //std::cout << "key : " << key << std::endl;
                if (lcpLength == currentNode.subText_.size())
                {
                    //std::cout << "PRIMA" << std::endl;
                    nextNodeIndex = currentNode.getChildIndex(key);
                    //std::cout << "nextNodeIndex : " << nextNodeIndex << std::endl;
                    if (nextNodeIndex != Node::INDEX_NULL)
                    {
                        assert(key == (Node::at(nextNodeIndex).key()));
                        from += lcpLength;
                        currentNodeIndex = nextNodeIndex;
                    }
                    else
                    {
                        //std::cout << "PRIMA 2" << std::endl;
                        std::pair<NodeIndex,Int> matchingKey = getKeyLongestPrefix(currentNode, key);
                        //std::cout << "matchingKey: " << matchingKey << " - " << (matchingKey == Node::INDEX_NULL) << std::endl;
                        //std::cout << "Node::at(matchingKey).value_: " << Node::at(matchingKey).value_ << std::endl;
                        return matchingKey.first == Node::INDEX_NULL
                                ? std::make_pair(-1,0)
                                : std::make_pair(Node::at(matchingKey.first).value_,Int(from)+matchingKey.second);
                    }
                }
                else { 
                        //std::cout << "lcpLength: " << lcpLength << std::endl;
                        Int lcpLength_ = LongString::getCharLCPLength(
                                currentNode.subText_.getLong(lcpLength), key);

                        return std::make_pair(currentNode.value_,
                                             (from+lcpLength)*LONG_PAR_CHAR+lcpLength_);
                     }
            }
        }
    }

    inline std::string getLongestMatchSeq(const std::string &pattern) const
    {
        return getLongestMatchSeq(LongString(&pattern));
    }

    inline std::string getLongestMatchSeq(const LongString &pattern) const
    {
        NodeIndex currentNodeIndex = root_;

        Int pattern_size = pattern.size();
        Int from = 0;
        Int lcpLength;
        Ulong key;
        NodeIndex nextNodeIndex;
        while (true) {
            const Node &currentNode = Node::at(currentNodeIndex);
            lcpLength =
                getLCPLength(currentNode.subText_, pattern, from, pattern_size);

            if (lcpLength == pattern_size - from) { return currentNode.subText_.text_; }
            else
            {
                key = pattern.getLong(lcpLength + from);
                if (lcpLength == currentNode.subText_.size())
                {
                    nextNodeIndex = currentNode.getChildIndex(key);
                    if (nextNodeIndex != Node::INDEX_NULL)
                    {
                        assert(key == (Node::at(nextNodeIndex).key()));
                        from += lcpLength;
                        currentNodeIndex = nextNodeIndex;
                    }
                    else
                    {
                        std::pair<NodeIndex,Int> matchingKey = getKeyLongestPrefix(currentNode, key);

                        return matchingKey.first == Node::INDEX_NULL
                                ? std::string("")
                                : Node::at(matchingKey.first).subText_.text_;
                    }
                }
                else { return currentNode.subText_.text_; }
            }
        }
    }

    inline void insertChild(Node &targetNode, const Ulong &key,
                            NodeIndex nodeIndex)
    {
        if (targetNode.children_ == nullptr) {
            targetNode.children_ =
                new typename Node::Word2NodeMap(1, Node::INDEX_NULL);
            targetNode.microTrieIndex_ = MicroTrie::make();
        }
        #ifdef USE_NEWDATASTRUCTURE
                targetNode.children_->insert(nodeIndex);
        #else
                (*targetNode.children_->)[key] = nodeIndex;
        #endif
        MicroTrie::at(targetNode.microTrieIndex_).insert(key, nodeIndex);
    }

    inline void eraseNode(NodeIndex parentNodeIndex, Node &targetNode) {
        targetNode.value_ = EMPTY_VALUE;
        auto key = targetNode.key();
        if (targetNode.children_ == nullptr) {
            if (parentNodeIndex == Node::INDEX_NULL) {
                Node::at(root_).set(LongString(), EMPTY_VALUE);
            } else {
                Node &parentNode = Node::at(parentNodeIndex);
                assert(parentNode.children_ != nullptr);
                parentNode.children_->erase(key);
                MicroTrie::at(parentNode.microTrieIndex_).erase(key);
                if (parentNode.children_->size() == 1 &&
                    parentNode.value_ == EMPTY_VALUE) {
                    // swap parent and child Node::at
                    Node &childNode =
                        Node::at(parentNode.children_->begin().value());
                    childNode.subText_.from_ = parentNode.subText_.from_;
                    parentNode.set(childNode.subText_, childNode.value_);
                    swapChildren(parentNode, childNode);
                    // delete parentNode
                    childNode.reset();
                }
            }
        } else if (targetNode.children_->size() == 1) {
            if (parentNodeIndex == Node::INDEX_NULL) {
            } else {
                Node &parentNode = Node::at(parentNodeIndex);
                MicroTrie &parentMicroTrie =
                    MicroTrie::at(parentNode.microTrieIndex_);
                assert(parentNode.children_ != nullptr);
                parentNode.children_->erase(key);
                auto iter = targetNode.children_->begin();
                assert(iter != targetNode.children_->end());
                NodeIndex childNodeIndex = iter.value();
                Node &childNode = Node::at(iter.value());
                childNode.subText_.from_ = targetNode.subText_.from_;
                parentNode.children_->insert(childNodeIndex);
                parentMicroTrie.update(childNode.key(), childNodeIndex);
                targetNode.reset();
            }
        } else {
            LOG("Should't erase node");
        }
    }

    inline bool containsKeyPrefix(const Node &node, const Ulong &key) const {
        return node.children_ == nullptr
                   ? false
                   : MicroTrie::at(node.microTrieIndex_).containsPrefix(key);
    }

    inline Ulong getKeyPrefix(const Node &node, const Ulong &key) const
    {
        return node.children_ == nullptr
                   ? -1
                   : MicroTrie::at(node.microTrieIndex_).getPrefix(key);
    }

    inline std::pair<NodeIndex,Int> 
                getKeyLongestPrefix(const Node &node, const Ulong &key) const
    {
        return node.children_ == nullptr
                   ? std::make_pair(Ulong(-1),0)
                   : MicroTrie::at(node.microTrieIndex_).getLongestPrefix(key);
    }

    inline void swapChildren(Node &a, Node &b) {
        std::swap(a.children_, b.children_);
        std::swap(a.microTrieIndex_, b.microTrieIndex_);
    }

    inline void print() { Node::at(root_).print(); }

    inline Ulong store(std::ofstream& out) { return Node::at(root_).store(out); }
    inline void load(std::ifstream& in) { Node::at(root_).load(in); }
};

template <typename Value, const Value EMPTY_VALUE>
typename CTriePP<Value, EMPTY_VALUE>::Node::NodeFactory
    CTriePP<Value, EMPTY_VALUE>::Node::nodeFactory_;

}  // namespace ctriepp

#endif  // C_TRIE_PP_HPP
