#pragma once
// https://github.com/shogo314/shogo-library/blob/24638d870d2ac03d01e1498411ab8aa477ea366f/src/utility/bit.hpp

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
