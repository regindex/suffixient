#ifndef FAST_HPP
#define FAST_HPP

// #include <city.h>
#include "any.hpp"

namespace ctriepp {

namespace Fast {
// public:
/// Returns the most significant bit of an integer.
///
/// Parameters:
///     x - an integer.
/// Returns:
///     the most significant bit of x, of x is nonzero; −1, otherwise.
inline static Int mostSignificantBit(const Ulong& x) {
    return x == 0 ? -1 : 63 - __builtin_clzll(x);
}

inline static Ulong twoFattest(const Ulong& a, const Ulong& b) {
    // Ulong _a = std::max(1LL, a);
    return a == b ? 0 : ((LONG_ALL_ONE << mostSignificantBit(a ^ b)) & b);
}

struct Hash {
    // Uint operator()(Ulong v) { return v * 7; }
    // /**
    //  * FNV Constants
    //  */
    // static const Uint FNV_OFFSET_BASIS_32 = 2166136261U;
    // static const Uint FNV_PRIME_32 = 16777619U;
    // Uint operator()(Ulong v) {
    //     Uint hash = FNV_OFFSET_BASIS_32;
    //     hash = (FNV_PRIME_32 * hash) ^ (Uint)v;
    //     hash = (FNV_PRIME_32 * hash) ^ (Uint)(v >> INT_SIZE);
    //     return hash;
    // }

    /*  Written in 2015 by Sebastiano Vigna (vigna@acm.org)

    To the extent possible under law, the author has dedicated all copyright
    and related and neighboring rights to this software to the public domain
    worldwide. This software is distributed without any warranty.

    See <http://creativecommons.org/publicdomain/zero/1.0/>. */

    /* This is a fixed-increment version of Java 8's SplittableRandom generator
       See http://dx.doi.org/10.1145/2714064.2660195 and
       http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html

       It is a very fast generator passing BigCrush, and it can be useful if
       for some reason you absolutely want 64 bits of state; otherwise, we
       rather suggest to use a xoroshiro128+ (for moderately parallel
       computations) or xorshift1024* (for massively parallel computations)
       generator. */
    inline Uint operator()(const Ulong& x) const {
        Ulong z = (x + 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    // inline Ulong operator()(const Uchar& x) const { return x; }

    // size_t operator()(Ulong v) {
    //     size_t result;
    //     Fast::MurmurHash3_x64_128((const void *)v, 64, 12741928,
    //                               (void *)result);
    //     return result;
    // }
};

// Equal comparison for using std::string as lookup key
struct Equal {
    inline bool operator()(const Ulong& lhs, const Ulong& rhs) {
        return lhs == rhs;
    }
};

struct UcharHash {
    inline Uint operator()(const Uchar& x) const {
        Ulong z = (0x9e3779b97f4a7c15ULL + x);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }
};

struct UcharEqual {
    inline bool operator()(const Uchar& lhs, const Uchar& rhs) {
        return lhs == rhs;
    }
};
};  // namespace Fast

// namespace city {
// struct hash {
//     inline Ulong operator()(const Ulong& key) const {
//         return CityHash64((const char*)&key, LONG_PAR_CHAR);
//     }
// };
// }  // namespace city

}  // namespace ctriepp

#endif  // FAST_HPP
