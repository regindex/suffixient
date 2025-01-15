#include "TFastTrie.hpp"
// #include "tsl/robin_map.h"

template <typename Value>
class TPackedTrie {
   public:
    class Node;
    typedef Factory<Node> NodeFactory;
    typedef typename NodeFactory::Index NodeIndex;
    typedef TFastTrie<NodeIndex> MicroTrie;
    typedef Factory<MicroTrie> MicroTrieFactory;
    typedef typename MicroTrieFactory::Index MicroTrieIndex;
    typedef typename MicroTrie::Node MicroTrieNode;
    typedef Factory<MicroTrieNode> MicroTrieNodeFactory;

    class Node {
       public:
        typedef rigtorp::HashMap<Ulong, NodeIndex, Fast::Hash, Fast::Equal>
            Word2NodeMap;

        Value value_;
        LongString subText_;
        Word2NodeMap *children_;
        MicroTrieIndex microTrieIndex_;

        inline Node()
            : subText_(),
              children_(nullptr),
              microTrieIndex_(MicroTrieFactory::INDEX_NULL) {}

        inline void set(LongString subLongString, const Value &value) {
            value_ = value;
            subText_ = subLongString;
        }

        inline bool hasChild(const Ulong &key) const {
            return children_ == nullptr
                       ? false
                       : children_->find(key) == children_->end() ? false
                                                                  : true;
        }

        inline void print() const {
            INFO("TPackedTrie Node");
            if (children_ == nullptr) {
                INFO("    nullptr");
            }
            auto iter = children_->begin();
            for (; iter != children_->end(); ++iter) {
                INFO(LongString::toString(iter->first));
            }
        }

        inline NodeIndex getChildIndex(const Ulong &key) const {
            if (children_ == nullptr) {
                return NodeFactory::INDEX_NULL;
            }
            auto iter = children_->find(key);
            return iter == children_->end() ? NodeFactory::INDEX_NULL
                                            : iter->second;
        }

        inline Ulong key() const {
            assert(0 < subText_.size());
            return subText_.getLong(0);
        }
    };

    NodeFactory nodeFactory_;
    MicroTrieFactory microTrieFactory_;
    MicroTrieNodeFactory microTrieNodeFactory_;
    NodeIndex root_;

   public:
    inline TPackedTrie() { root_ = NodeFactory::INDEX_NULL; }

    inline void add(const std::string *newText, const Value &value) {
        add(LongString(newText), value);
    }

    inline void add(const LongString &newText, const Value &value) {
        // LOG("add new text into TPackedTrie : " << _newText);
        LongString newSubText = LongString(newText);

        if (root_ == NodeFactory::INDEX_NULL) {
            root_ = makeNodeIndex();
            node(root_).set(newSubText, value);
        } else {
            insert(root_, newSubText, value);
        }
        assert(hasPrefix(newText));
    }

    inline void insert(NodeIndex targetNodeIndex, const LongString newSubText,
                       const Value &value) {
        Node &targetNode = node(targetNodeIndex);
        Int lcpLength = getLCPLength(newSubText, targetNode.subText_);
        NodeIndex nextNodeIndex;
        if (lcpLength < newSubText.size()) {
            LongString newLastText =
                LongString(newSubText, lcpLength, newSubText.size());
            Ulong key = newSubText.getLong(lcpLength);
            if (lcpLength == targetNode.subText_.size()) {
                nextNodeIndex = targetNode.getChildIndex(key);
                if (nextNodeIndex != NodeFactory::INDEX_NULL) {
                    insert(nextNodeIndex, newLastText, value);
                } else {
                    NodeIndex newNodeIndex = makeNodeIndex();
                    node(newNodeIndex).set(newLastText, value);
                    addChild(targetNode, key, newNodeIndex);
                }
            } else {
                NodeIndex newNodeIndex = makeNodeIndex();
                Node &newNode = node(newNodeIndex);
                newNode.set(LongString(targetNode.subText_, lcpLength,
                                       targetNode.subText_.size()),
                            targetNode.value_);
                swapChildren(targetNode, newNode);
                targetNode.subText_.to_ = targetNode.subText_.from_ + lcpLength;
                addChild(targetNode, newNode.subText_.getLong(0), newNodeIndex);

                newNodeIndex = makeNodeIndex();
                node(newNodeIndex).set(newLastText, value);
                addChild(targetNode, key, newNodeIndex);
            }
        }

        //     ERR("add same string.");
        //     assert(false);
        // }
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

    // Ulong getLong(const LongString &subLongString, Int index) const {
    //     assert(0 <= index);
    //     assert(index < subLongString.size());
    //     return subLongString.base_.getLong(subLongString.from_ + index);
    // }

    inline bool hasPrefix(const std::string *pattern) const {
        LOG("hasPrefix " << pattern);
        // LongString *pattern = new ;
        return hasPrefix(LongString(pattern));
    }

    inline bool hasPrefix(const std::string pattern) const {
        return hasPrefix(&pattern);
    }

    inline bool hasPrefix(const LongString &pattern) const {
        NodeIndex currentNodeIndex = root_;

        Int pattern_size = pattern.size();
        Int from = 0;
        Int lcpLength;
        Ulong key;
        NodeIndex nextNodeIndex;
        while (true) {
            const Node &currentNode = node(currentNodeIndex);
            lcpLength =
                getLCPLength(currentNode.subText_, pattern, from, pattern_size);
            // LOG("LCP Length : " << lcpLength);
            if (lcpLength == pattern_size - from) {
                return true;
            } else {
                key = pattern.getLong(lcpLength + from);
                if (currentNode.subText_.size() != 0) {
                }
                if (lcpLength == currentNode.subText_.size()) {
                    nextNodeIndex = currentNode.getChildIndex(key);
                    if (nextNodeIndex != NodeFactory::INDEX_NULL) {
                        assert(key == (node(nextNodeIndex).key()));
                        from += lcpLength;
                        currentNodeIndex = nextNodeIndex;
                    } else {
                        return hasKeyPrefix(currentNode, key);
                    }
                } else {
                    return LongString::isCharPrefix(
                        currentNode.subText_.getLong(lcpLength), key);
                }
            }
        }
    }

    inline void addChild(Node &targetNode, const Ulong &key,
                         NodeIndex nodeIndex) {
        if (targetNode.children_ == nullptr) {
            targetNode.children_ =
                new typename Node::Word2NodeMap(1, NodeFactory::INDEX_NULL);
            targetNode.microTrieIndex_ = makeMicroTrieIndex();
            microTrie(targetNode.microTrieIndex_).set(&microTrieNodeFactory_);
        }
        (*targetNode.children_)[key] = nodeIndex;
        microTrie(targetNode.microTrieIndex_).add(key, nodeIndex);
    }

    inline bool hasKeyPrefix(const Node &node, const Ulong &key) const {
        return node.children_ == nullptr
                   ? false
                   : microTrie(node.microTrieIndex_).hasPrefix(key);
    }

    inline void swapChildren(Node &a, Node &b) {
        std::swap(a.children_, b.children_);
        std::swap(a.microTrieIndex_, b.microTrieIndex_);
    }

    inline MicroTrieIndex makeMicroTrieIndex() {
        return microTrieFactory_.make();
    }

    inline MicroTrie &microTrie(const MicroTrieIndex &index) {
        return microTrieFactory_.at(index);
    }

    inline const MicroTrie &microTrie(const MicroTrieIndex &index) const {
        return microTrieFactory_.at(index);
    }

    inline NodeIndex makeNodeIndex() { return nodeFactory_.make(); }

    inline Node &node(const NodeIndex &index) { return nodeFactory_.at(index); }

    inline const Node &node(const NodeIndex &index) const {
        return nodeFactory_.at(index);
    }
};
