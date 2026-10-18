#pragma once
// https://github.com/shogo314/shogo-library/blob/db9a2f114afe8539fdb02e5a44fc654c2297c5fa/src/utility/bit.hpp

/**
 * @file bit.hpp
 * @brief ビット演算
 */

#include <cassert>
#include <string>

/**
 * @brief 2進数の文字列をlong longにする
 */
inline long long btoll(std::string s, char one = '1') {
    long long res = 0;
    for (char c : s) {
        res <<= 1;
        if (c == one) ++res;
    }
    return res;
}

#if __cplusplus < 202000L

// C++20 の <bit> と同じ値を返す (0 のときも同じ)

/**
 * @brief 立っているビットを数える
 * __builtin_popcountll
 */
inline constexpr long long popcount(long long a) {
    assert(a >= 0);
    return __builtin_popcountll((unsigned long long)a);
}
/**
 * @brief 左から連続した0のビットを数える
 * __builtin_clzll
 */
inline constexpr long long countl_zero(long long a) {
    assert(a >= 0);
    return a == 0 ? 64 : __builtin_clzll((unsigned long long)a);
}
/**
 * @brief 左から連続した1のビットを数える
 */
inline constexpr long long countl_one(long long a) {
    assert(a >= 0);
    unsigned long long x = ~(unsigned long long)a;
    return x == 0 ? 64 : __builtin_clzll(x);
}
/**
 * @brief 右から連続した0のビットを数える
 * __builtin_ctzll
 */
inline constexpr long long countr_zero(long long a) {
    assert(a >= 0);
    return a == 0 ? 64 : __builtin_ctzll((unsigned long long)a);
}
/**
 * @brief 右から連続した1のビットを数える
 */
inline constexpr long long countr_one(long long a) {
    assert(a >= 0);
    unsigned long long x = ~(unsigned long long)a;
    return x == 0 ? 64 : __builtin_ctzll(x);
}
/**
 * @brief 値を表現するために必要なビット幅を求める
 */
inline constexpr long long bit_width(long long a) {
    assert(a >= 0);
    return 64 - countl_zero(a);
}
/**
 * @brief 整数値を2の累乗値に切り上げる
 */
inline constexpr long long bit_ceil(long long a) {
    assert(0 <= a && a <= (1LL << 62));
    return a <= 1 ? 1 : 1LL << bit_width(a - 1);
}
/**
 * @brief 整数値を2の累乗値に切り下げる
 */
inline constexpr long long bit_floor(long long a) {
    assert(a >= 0);
    return a == 0 ? 0 : 1LL << (bit_width(a) - 1);
}

#else

#include <bit>
#define BIT_FUNC_EXPAND(func)                      \
    inline constexpr long long func(long long a) { \
        assert(a >= 0);                            \
        return std::func((unsigned long long)a);   \
    }

/**
 * @brief 立っているビットを数える
 */
BIT_FUNC_EXPAND(popcount)
/**
 * @brief 左から連続した0のビットを数える
 */
BIT_FUNC_EXPAND(countl_zero)
/**
 * @brief 左から連続した1のビットを数える
 */
BIT_FUNC_EXPAND(countl_one)
/**
 * @brief 右から連続した0のビットを数える
 */
BIT_FUNC_EXPAND(countr_zero)
/**
 * @brief 右から連続した1のビットを数える
 */
BIT_FUNC_EXPAND(countr_one)
/**
 * @brief 整数値を2の累乗値に切り上げる
 */
inline constexpr long long bit_ceil(long long a) {
    // 結果が long long に収まる範囲に限る
    assert(0 <= a && a <= (1LL << 62));
    return std::bit_ceil((unsigned long long)a);
}
/**
 * @brief 整数値を2の累乗値に切り下げる
 */
BIT_FUNC_EXPAND(bit_floor)
/**
 * @brief 値を表現するために必要なビット幅を求める
 */
BIT_FUNC_EXPAND(bit_width)

#endif

/**
 * @brief 2 つの整数のハミング距離 (ビットが異なる位置の数)
 */
inline constexpr long long hamming_distance(long long a, long long b) {
    assert(a >= 0 && b >= 0);
    return popcount(a ^ b);
}

namespace internal_bit {
// 範囲 for で使う列挙。next で次の値に進み、done になったら終わる
template <class Next>
struct BitEnumeration {
    long long first;
    bool empty;
    Next next;  // next(s) は次の値、終わりなら -1
    struct iterator {
        long long s;
        const Next* next;
        constexpr long long operator*() const { return s; }
        constexpr iterator& operator++() {
            s = (*next)(s);
            return *this;
        }
        constexpr bool operator!=(const iterator& other) const { return s != other.s; }
        constexpr bool operator==(const iterator& other) const { return s == other.s; }
    };
    constexpr iterator begin() const { return {empty ? -1 : first, &next}; }
    constexpr iterator end() const { return {-1, &next}; }
};
template <class Next>
constexpr BitEnumeration<Next> make_bit_enumeration(long long first, bool empty, Next next) {
    return {first, empty, next};
}
}  // namespace internal_bit

/**
 * @brief mask の部分集合を mask から 0 まで降順に列挙する (0 と mask を含む) 全体で O(2^popcount(mask))
 */
inline constexpr auto subsets(long long mask) {
    assert(mask >= 0);
    return internal_bit::make_bit_enumeration(mask, false, [mask](long long s) { return s == 0 ? -1 : (s - 1) & mask; });
}
/**
 * @brief n ビットの集合のうち mask を含むものを mask から昇順に列挙する (mask と 2^n - 1 を含む) 全体で O(2^(n - popcount(mask)))
 */
inline constexpr auto supersets(long long mask, long long n) {
    assert(0 <= n && n <= 62 && 0 <= mask && mask < (1LL << n));
    long long full = (1LL << n) - 1;
    return internal_bit::make_bit_enumeration(mask, false, [mask, full](long long s) { return s == full ? -1 : (s + 1) | mask; });
}
/**
 * @brief mask の立っているビットの位置を昇順に列挙する 全体で O(popcount(mask))
 */
inline constexpr auto bits(long long mask) {
    assert(mask >= 0);
    // 内部では残りのビットを持ち、* で最下位ビットの位置を返す
    struct Bits {
        long long mask;
        struct iterator {
            long long s;
            constexpr long long operator*() const { return countr_zero(s); }
            constexpr iterator& operator++() {
                s &= s - 1;
                return *this;
            }
            constexpr bool operator!=(const iterator& other) const { return s != other.s; }
            constexpr bool operator==(const iterator& other) const { return s == other.s; }
        };
        constexpr iterator begin() const { return {mask}; }
        constexpr iterator end() const { return {0}; }
    };
    return Bits{mask};
}
/**
 * @brief n ビットの集合のうち、ちょうど k 個のビットが立つものを昇順に列挙する (Gosper's hack) 全体で O(C(n, k))
 * k > n なら何も列挙しない
 */
inline constexpr auto combinations_mask(long long n, long long k) {
    assert(0 <= n && n <= 62 && 0 <= k);
    long long limit = 1LL << n;
    return internal_bit::make_bit_enumeration(k > n ? -1 : (1LL << k) - 1, k > n, [limit](long long s) {
        if (s == 0) return -1LL;  // k = 0 のときは 0 だけ
        long long c = s & -s, r = s + c;
        long long t = (((r ^ s) >> 2) / c) | r;
        return t >= limit ? -1 : t;
    });
}
