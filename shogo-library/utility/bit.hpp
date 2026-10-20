#pragma once
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/utility/bit.hpp

/**
 * @file bit.hpp
 * @brief ビット演算
 */

#include <cassert>
#include <string>
#include <utility>

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
    return __builtin_popcountll(static_cast<unsigned long long>(a));
}
/**
 * @brief 左から連続した0のビットを数える
 * __builtin_clzll
 */
inline constexpr long long countl_zero(long long a) {
    assert(a >= 0);
    return a == 0 ? 64 : __builtin_clzll(static_cast<unsigned long long>(a));
}
/**
 * @brief 左から連続した1のビットを数える
 */
inline constexpr long long countl_one(long long a) {
    assert(a >= 0);
    unsigned long long x = ~static_cast<unsigned long long>(a);
    return x == 0 ? 64 : __builtin_clzll(x);
}
/**
 * @brief 右から連続した0のビットを数える
 * __builtin_ctzll
 */
inline constexpr long long countr_zero(long long a) {
    assert(a >= 0);
    return a == 0 ? 64 : __builtin_ctzll(static_cast<unsigned long long>(a));
}
/**
 * @brief 右から連続した1のビットを数える
 */
inline constexpr long long countr_one(long long a) {
    assert(a >= 0);
    unsigned long long x = ~static_cast<unsigned long long>(a);
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
/**
 * @brief 2 の累乗か (ちょうど 1 つのビットが立っているか)
 */
inline constexpr bool has_single_bit(long long a) {
    assert(a >= 0);
    return a != 0 && (a & (a - 1)) == 0;
}

#else

#include <bit>
#define BIT_FUNC_EXPAND(func)                                 \
    inline constexpr long long func(long long a) {            \
        assert(a >= 0);                                       \
        return std::func(static_cast<unsigned long long>(a)); \
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
    return std::bit_ceil(static_cast<unsigned long long>(a));
}
/**
 * @brief 整数値を2の累乗値に切り下げる
 */
BIT_FUNC_EXPAND(bit_floor)
/**
 * @brief 値を表現するために必要なビット幅を求める
 */
BIT_FUNC_EXPAND(bit_width)
/**
 * @brief 2 の累乗か (ちょうど 1 つのビットが立っているか)
 */
inline constexpr bool has_single_bit(long long a) {
    assert(a >= 0);
    return std::has_single_bit(static_cast<unsigned long long>(a));
}

#endif

/**
 * @brief 2 つの整数のハミング距離 (ビットが異なる位置の数)
 */
inline constexpr long long hamming_distance(long long a, long long b) {
    assert(a >= 0 && b >= 0);
    return popcount(a ^ b);
}

/**
 * @brief 最上位の立っているビットの位置 (0 なら -1)
 */
inline constexpr long long msb(long long a) { return bit_width(a) - 1; }
/**
 * @brief 最下位の立っているビットの位置 (0 なら -1)
 */
inline constexpr long long lsb(long long a) {
    assert(a >= 0);
    return a == 0 ? -1 : countr_zero(a);
}
/**
 * @brief 最下位の立っているビットだけを残した値 a & -a (0 なら 0)
 */
inline constexpr long long lowbit(long long a) {
    assert(a >= 0);
    return a & -a;
}
/**
 * @brief a の i ビット目が立っているか
 */
inline constexpr bool test_bit(long long a, long long i) {
    assert(a >= 0 && 0 <= i && i < 63);
    return a >> i & 1;
}
/**
 * @brief a の i ビット目を 1 にした値
 */
inline constexpr long long set_bit(long long a, long long i) {
    assert(a >= 0 && 0 <= i && i < 63);
    return a | 1LL << i;
}
/**
 * @brief a の i ビット目を 0 にした値
 */
inline constexpr long long reset_bit(long long a, long long i) {
    assert(a >= 0 && 0 <= i && i < 63);
    return a & ~(1LL << i);
}
/**
 * @brief a の i ビット目を反転した値
 */
inline constexpr long long flip_bit(long long a, long long i) {
    assert(a >= 0 && 0 <= i && i < 63);
    return a ^ 1LL << i;
}
/**
 * @brief 立っているビットの数の偶奇 (popcount(a) % 2)
 */
inline constexpr long long parity(long long a) { return popcount(a) & 1; }
/**
 * @brief long long を n 桁の 2 進数の文字列にする (btoll の逆)
 * n を省略すると bit_width(a) 桁 (0 なら "0")
 */
inline std::string lltob(long long a, long long n = -1, char one = '1', char zero = '0') {
    assert(a >= 0);
    if (n < 0) n = a == 0 ? 1 : bit_width(a);
    assert(n >= bit_width(a));
    std::string res(n, zero);
    for (long long i = 0; i < n && i < 63; i++) {
        if (a >> i & 1) res[n - 1 - i] = one;
    }
    return res;
}
/**
 * @brief 下位 n ビットの並びを逆にする (a < 2^n)
 */
inline constexpr long long reverse_bits(long long a, long long n) {
    assert(0 <= n && n <= 63 && 0 <= a && bit_width(a) <= n);
    long long res = 0;
    for (long long i = 0; i < n; i++) {
        if (a >> i & 1) res |= 1LL << (n - 1 - i);
    }
    return res;
}
/**
 * @brief 下位 n ビットを左に k ビット巡回シフトする (a < 2^n、k は負でもよい)
 */
inline constexpr long long rotate_left(long long a, long long k, long long n) {
    assert(1 <= n && n <= 63 && 0 <= a && bit_width(a) <= n);
    k %= n;
    if (k < 0) k += n;
    if (k == 0) return a;
    unsigned long long full = (1ULL << n) - 1;
    return static_cast<long long>((static_cast<unsigned long long>(a) << k & full) | static_cast<unsigned long long>(a) >> (n - k));
}
/**
 * @brief 下位 n ビットを右に k ビット巡回シフトする (a < 2^n、k は負でもよい)
 */
inline constexpr long long rotate_right(long long a, long long k, long long n) {
    assert(1 <= n && n <= 63);
    return rotate_left(a, -(k % n), n);
}
/**
 * @brief グレイコード (隣り合う値で 1 ビットだけ変わる) a ^ (a >> 1)
 */
inline constexpr long long gray_code(long long a) {
    assert(a >= 0);
    return a ^ a >> 1;
}
/**
 * @brief グレイコードから元の値に戻す (gray_code の逆)
 */
inline constexpr long long inverse_gray_code(long long g) {
    assert(g >= 0);
    for (long long s = 1; s < 64; s <<= 1) g ^= g >> s;
    return g;
}
/**
 * @brief mask の立っているビットのうち、下から k 番目 (0-indexed) の位置 O(log 64)
 */
inline constexpr long long kth_set_bit(long long mask, long long k) {
    assert(mask >= 0 && 0 <= k && k < popcount(mask));
    long long res = 0;
    for (long long w = 32; w > 0; w >>= 1) {
        // 下位 w ビットに立っているビットの数で、答えが上下どちらにあるかを決める
        long long c = popcount(mask & ((1LL << w) - 1));
        if (k >= c) {
            k -= c;
            mask >>= w;
            res += w;
        }
    }
    return res;
}
/**
 * @brief 0 ^ 1 ^ ... ^ n
 */
inline constexpr long long xor_upto(long long n) {
    assert(n >= 0);
    // 4 つごとに周期的になる
    long long r = n & 3;
    // r ごとの値の表として読めるように 1 行で書いているので整形しない
    // clang-format off
    return r == 0 ? n : r == 1 ? 1 : r == 2 ? n + 1 : 0;
    // clang-format on
}
/**
 * @brief 0 以上 n 以下の整数のうち、i ビット目が立っているものの数
 */
inline constexpr long long count_bit_upto(long long n, long long i) {
    assert(0 <= n && n < 0x7fffffffffffffffLL && 0 <= i && i < 63);
    // 2^(i + 1) ごとに、後半の 2^i 個で立つ
    unsigned long long m = static_cast<unsigned long long>(n) + 1, half = 1ULL << i, cycle = half << 1;
    unsigned long long r = m % cycle;
    return static_cast<long long>(m / cycle * half + (r > half ? r - half : 0));
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
 * @brief s と立っているビットの数が同じで、s より大きい最小の値 (Gosper's hack)
 * long long に収まらなければ -1
 */
inline constexpr long long next_combination_mask(long long s) {
    assert(s > 0);
    unsigned long long u = s, c = u & -u, r = u + c;
    unsigned long long t = (((r ^ u) >> 2) / c) | r;
    return t > 0x7fffffffffffffffULL ? -1 : static_cast<long long>(t);
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
        long long t = next_combination_mask(s);
        return t >= limit ? -1 : t;
    });
}
/**
 * @brief mask の部分集合のうち、ちょうど k 個のビットが立つものを昇順に列挙する 1 つあたり O(k)
 * k > popcount(mask) なら何も列挙しない
 */
inline constexpr auto subsets_of_size(long long mask, long long k) {
    assert(mask >= 0 && 0 <= k);
    // mask の下位 cnt 個の立っているビット
    auto lowest = [mask](long long cnt) {
        long long res = 0, rest = mask;
        for (long long j = 0; j < cnt; j++) {
            res |= rest & -rest;
            rest &= rest - 1;
        }
        return res;
    };
    bool empty = k > popcount(mask);
    return internal_bit::make_bit_enumeration(empty ? -1 : lowest(k), empty, [mask, k, lowest](long long s) {
        if (s == 0) return -1LL;  // k = 0 のときは 0 だけ
        // mask の外のビットを 1 とみなして最下位ビットを足すと、繰り上がりが mask の中だけを進む
        unsigned long long c = s & -s, r = ((static_cast<unsigned long long>(s) | ~static_cast<unsigned long long>(mask)) + c) & static_cast<unsigned long long>(mask);
        if (r == 0) return -1LL;  // 繰り上がりが mask の最上位を越えた
        // 繰り上がりで消えたビットを mask の下位に詰め直す
        return static_cast<long long>(r) | lowest(k - popcount(static_cast<long long>(r)));
    });
}

namespace internal_bit {
// rest の立っているビット i を昇順に、(i, mask ^ (1 << i)) の組で列挙する
struct ToggleOneBit {
    long long mask, rest;
    struct iterator {
        long long mask, s;
        constexpr std::pair<long long, long long> operator*() const {
            long long i = countr_zero(s);
            return {i, mask ^ (1LL << i)};
        }
        constexpr iterator& operator++() {
            s &= s - 1;
            return *this;
        }
        constexpr bool operator!=(const iterator& other) const { return s != other.s; }
        constexpr bool operator==(const iterator& other) const { return s == other.s; }
    };
    constexpr iterator begin() const { return {mask, rest}; }
    constexpr iterator end() const { return {mask, 0}; }
};
}  // namespace internal_bit

/**
 * @brief n ビットのうち mask に含まれない位置 i を昇順に、(i, mask | (1 << i)) の組で列挙する 全体で O(n - popcount(mask))
 * bit DP で 1 つ要素を足す遷移に使う
 */
inline constexpr internal_bit::ToggleOneBit add_one(long long mask, long long n) {
    assert(0 <= n && n <= 62 && 0 <= mask && mask < (1LL << n));
    return {mask, ((1LL << n) - 1) ^ mask};
}
/**
 * @brief mask に含まれる位置 i を昇順に、(i, mask ^ (1 << i)) の組で列挙する 全体で O(popcount(mask))
 * bit DP で 1 つ要素を除いた集合から遷移するときに使う
 */
inline constexpr internal_bit::ToggleOneBit remove_one(long long mask) {
    assert(mask >= 0);
    return {mask, mask};
}

/**
 * @brief n ビットのすべての集合 S と、その部分集合 T の組 (S, T) を列挙する 全体で O(3^n)
 * S は 0 から昇順、各 S の中で T は S から 0 まで降順
 */
inline constexpr auto subset_pairs(long long n) {
    assert(0 <= n && n <= 62);
    struct SubsetPairs {
        long long n;
        struct iterator {
            long long s, t;
            constexpr std::pair<long long, long long> operator*() const { return {s, t}; }
            constexpr iterator& operator++() {
                if (t == 0) {
                    t = ++s;
                } else {
                    t = (t - 1) & s;
                }
                return *this;
            }
            constexpr bool operator!=(const iterator& other) const { return s != other.s || t != other.t; }
            constexpr bool operator==(const iterator& other) const { return s == other.s && t == other.t; }
        };
        constexpr iterator begin() const { return {0, 0}; }
        constexpr iterator end() const { return {1LL << n, 1LL << n}; }
    };
    return SubsetPairs{n};
}
