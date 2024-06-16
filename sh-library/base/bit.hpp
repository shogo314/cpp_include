#pragma once
#include <cassert>
#include <string>
#include <vector>

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
