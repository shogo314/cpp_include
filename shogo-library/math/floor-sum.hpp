#pragma once
// https://github.com/shogo314/shogo-library/blob/93de89301cdeac655ce53761092d9556752fd185/src/math/floor-sum.hpp

/**
 * @file floor-sum.hpp
 * @brief floor sum (一次関数の床関数の和)
 */

#include <cassert>
#include <utility>

namespace internal_floor_sum {
// a, b, m, n がすべて非負のとき。途中の値は 2^64 で割った余りで計算する
inline unsigned long long floor_sum_unsigned(unsigned long long n, unsigned long long m, unsigned long long a,
                                             unsigned long long b) {
    unsigned long long res = 0;
    while (true) {
        if (a >= m) {
            res += n * (n - 1) / 2 * (a / m);
            a %= m;
        }
        if (b >= m) {
            res += n * (b / m);
            b %= m;
        }
        unsigned long long y_max = a * n + b;
        if (y_max < m) break;
        // y = (a x + b) / m の格子点を、x と y を入れ替えて数え直す
        n = y_max / m;
        b = y_max % m;
        std::swap(m, a);
    }
    return res;
}
}  // namespace internal_floor_sum

/**
 * @brief sum_{i=0}^{n-1} floor((a i + b) / m) を返す O(log m)
 *
 * a, b は負でもよい。答えと途中の値 (a, b を [0, m) に直した後の a n + b など) が long long に収まること
 * @param n 0 以上
 * @param m 1 以上
 */
inline long long floor_sum(long long n, long long m, long long a, long long b) {
    assert(0 <= n && 1 <= m);
    unsigned long long res = 0;
    if (a < 0) {
        long long a2 = a % m + m;
        res -= static_cast<unsigned long long>(n) * (n - 1) / 2 * ((a2 - a) / m);
        a = a2;
    }
    if (b < 0) {
        long long b2 = b % m + m;
        res -= static_cast<unsigned long long>(n) * ((b2 - b) / m);
        b = b2;
    }
    return res + internal_floor_sum::floor_sum_unsigned(n, m, a, b);
}
