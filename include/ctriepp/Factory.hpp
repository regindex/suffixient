#ifndef FACTORY_HPP
#define FACTORY_HPP

#include <array>
#include <queue>
#include "Fast.hpp"

namespace ctriepp {

template <typename Value>
class Factory {
   public:
    constexpr static Uint ROW_SIZE = 256;
    typedef std::array<Value, ROW_SIZE> Row;
    typedef std::vector<Row*> Table;
    typedef uint8_t RowIndex;
    typedef Uint Index;

    constexpr static Index INDEX_NULL = -1;

    Table table_;
    Uint size_;
    std::queue<Index> erased_indeces_;

    inline Factory() : size_(0) {}

    inline const size_t size() { return size_ - erased_indeces_.size(); }

    inline const Index make() {
        if (table_.size() <= tableIndex(size_)) {
            table_.push_back(new Row());
        }
        if (erased_indeces_.empty()) {
            return size_++;
        } else {
            Index index = erased_indeces_.front();
            erased_indeces_.pop();
            return index;
        }
    }

    inline RowIndex rowIndex(const Index& index) const {
        return RowIndex(index);
    }

    inline Uint tableIndex(const Index& index) const { return DIV256(index); }

    inline Value& at(const Index& index) {
        assert(tableIndex(index) < table_.size());
        return table_[tableIndex(index)]->at(rowIndex(index));
    }

    inline const Value& at(const Index& index) const {
        assert(index < size_);
        return table_[tableIndex(index)]->at(rowIndex(index));
    }

    inline void erase(const Index& index) {
        assert(index < size_);
        erased_indeces_.push(index);
    }
};

}  // namespace ctriepp

#endif  // FACTORY_HPP
