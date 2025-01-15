#include "Factory.hpp"

namespace ctriepp {

class LongString
{
    static const Uchar UCHAR_MASK = 0xFF;

    public:
        std::string text_;

        inline LongString() : text_("") {}

        inline LongString(const std::string *text) : text_(*text) { assert(text_.size() > 0); }
        inline LongString(const LongString &longString): text_(longString.text_){ text_ = longString.text_; }

        inline LongString(const LongString &longString, const Uint &from,
                          const Uint &to)
        {
            assert(0 <= from);
            assert(from < to);
            text_ = longString.text_.substr(from * LONG_PAR_CHAR,
                                                to * LONG_PAR_CHAR);
        }

    inline void to(const Uint &to_) { text_.resize(to_ * LONG_PAR_CHAR); }

    inline Uint size() const { return std::ceil( double(text_.size()) / LONG_PAR_CHAR); }

    inline Ulong getLong(const Uint &index) const
    {
        assert(text_.size() > 0);

        if (text_.size() == 0) {
            ERROR("text_ == nullptr at LongString::getLong");
            return 0;
        } else if (index < (text_.size() - 1) / LONG_PAR_CHAR) {
            return ((const Ulong *)(text_.data()))[index];
        } else {
            return ((const Ulong *)(text_.data()))[index] &
                   PREFIX_MASK[(text_.size() - 1) % LONG_PAR_CHAR + 1];
        }
    }

    // is b prefix of a.
    inline static Int isCharPrefix(const Ulong &a, const Ulong &b) { 
        return getCharLCPLength(a, b) == charSize(b);
    }

    // get prefix length between b and a
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

    inline std::string toString() const { return text_; }

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
}
// namespace ctriepp