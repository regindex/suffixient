#include "Factory.hpp"

namespace ctriepp {

class LongString {
    static const Uchar UCHAR_MASK = 0xFF;
    // static const Ulong ALL_MASK = 0xFFFFFFFFFFFFFFFFLL;

    // std::vector<Ulong> bits_;
   public:
    const std::string *text_;
    Uint from_;
    Uint to_;

    inline LongString() : text_(nullptr), from_(0), to_(0) {}

    inline LongString(const std::string *text) : text_(text), from_(0) {
        assert(text != nullptr);
        to_ = text->size() == 0 ? 0 : ((text->size() - 1) / LONG_PAR_CHAR) + 1;
    }

    inline LongString(const LongString &longString)
        : text_(longString.text_),
          from_(longString.from_),
          to_(longString.to_) {}

    inline LongString(const LongString &longString, const Uint &from,
                      const Uint &to)
        : text_(longString.text_) {
        from_ = longString.from_ + from;
        to_ = longString.from_ + to;
        assert(0 <= from_);
        assert(from_ < to_);
        assert(to_ <= longString.to_);
    }

    inline Uint size() const { return to_ - from_; }

    inline Ulong getLong(const Uint &index) const {
        assert(text_ != nullptr);
        assert(from_ + index < to_);

        if (text_ == nullptr) {
            ERROR("text_ == nullptr at LongString::getLong");
            return 0;
        } else if (from_ + index < (text_->size() - 1) / LONG_PAR_CHAR) {
            return ((const Ulong *)(text_->data()))[from_ + index];
        } else {
            return ((const Ulong *)(text_->data()))[from_ + index] &
                   PREFIX_MASK[(text_->size() - 1) % LONG_PAR_CHAR + 1];
        }
    }

    // bool operator==(const LongString &a) const {
    //     // if (bits_.size() != a.bits_.size()) {
    //     //     return false;
    //     // }

    //     // for (Int i = 0; i < bits_.size(); ++i) {
    //     //     if (bits_[i] != a.bits_[i]) {
    //     //         return false;
    //     //     }
    //     // }
    //     // return true;
    // }

    // bool operator!=(const LongString &a) const { return !((*this) == a);
    // }

    // is b prefix of a.
    inline static Int isCharPrefix(const Ulong &a, const Ulong &b) {
        return getCharLCPLength(a, b) == charSize(b);
    }

    inline static Int getCharLCPLength(const Ulong &a, const Ulong &b) {
        if (a == b) {
            return charSize(a);
        }
        return __builtin_ctzll(a ^ b) >> 3;
    }

    inline static Ulong toSubLong(const Ulong &a, const Uint &from,
                                  const Uint &to) {
        assert(0 <= from);
        assert(from < to);
        assert(to <= charSize(a));
        return (a >> (from * CHAR_SIZE)) & PREFIX_MASK[to - from];
        // return ((a >> (from * CHAR_SIZE)) &
        //         ~(ALL_MASK << ((to - from) * CHAR_SIZE)));
        // return ((a ^ (a & (ALL_MASK << (to * CHAR_SIZE)))) >>
        //         (from * CHAR_SIZE));
    }

    inline static Ulong toLong(const std::string &a, const Uint &from = 0) {
        assert(0 <= from);
        assert(from < a.size());

        Uint to = std::min(from + LONG_PAR_CHAR, (Uint)a.size());

        Ulong result = 0;
        for (Uint i = from; i < to; ++i) {
            assert(0 != a[i]);
            if (a[i] == 0) {
                ERROR("Invalid char" << a[i]);
            }
            result |= (((Ulong)(Uchar)a[i]) << (i * CHAR_SIZE));
        }

        // Ulong result = *(Ulong *)a.c_str();
        return result;
    }

    inline static Uchar toChar(const Ulong &a, const Int &index) {
        assert(0 <= index);
        assert(index < LONG_PAR_CHAR);
        return (a >> (index * CHAR_SIZE)) & UCHAR_MASK;
    }

    inline std::string toString() const {
        if (text_ == nullptr) {
            return "";
        } else {
            return (*text_).substr(from_ * LONG_PAR_CHAR,
                                   size() * LONG_PAR_CHAR);
        }
        // std::string result;
        // for (size_t i = 0; i < size_; ++i) {
        //     result += toString(bits_[i]);
        // }
        // return result;
    }

    inline static std::string toString(const Ulong &a) {
        std::string result(charSize(a), '\0');
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = toChar(a, i);
        }
        return result;
    }

    inline static Uchar charSize(const Ulong &a) {
        return a == 0 ? 0
                      : (Fast::mostSignificantBit(a) + CHAR_SIZE) / CHAR_SIZE;
    }
};

class SubLongString {
   public:
    // Int text_id_;

    // const LongString *longString_;
    // SubLongString(const LongString *longString, Int from, Int to)
    //     : longString_(longString) {
    //     from_ = from;
    //     to_ = to;
    // }
};
}  // namespace ctriepp
