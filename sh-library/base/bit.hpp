#pragma once
#include <cassert>
#include <string>
#include <vector>

/**
 * @brief 2進数の文字列をlong longにする
 */
long long btoll(std::string s, char one = '1') {
    long long res = 0;
    for (char c : s) {
        res <<= 1;
        if (c == one) ++res;
    }
    return res;
}

#if __cplusplus < 202000L

/**
 * @brief 立っているビットを数える
 * __builtin_popcountll
 */
int popcount(long long a) {
    assert(a >= 0);
    return __builtin_popcountll((unsigned long long)a);
}
/**
 * @brief 左から連続した0のビットを数える
 * __builtin_clzll
 */
int countl_zero(long long a) {
    assert(a >= 0);
    return __builtin_clzll((unsigned long long)a);
}
/**
 * @brief 右から連続した0のビットを数える
 * __builtin_ctzll
 */
int countr_zero(long long a) {
    assert(a >= 0);
    return __builtin_ctzll((unsigned long long)a);
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
BIT_FUNC_EXPAND(bit_ceil)
/**
 * @brief 整数値を2の累乗値に切り下げる
 */
BIT_FUNC_EXPAND(bit_floor)
/**
 * @brief 値を表現するために必要なビット幅を求める
 */
BIT_FUNC_EXPAND(bit_width)

#endif
