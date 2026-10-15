#pragma once
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/math/sqrt.hpp

/**
 * @file sqrt.hpp
 * @brief 整数の平方根
 */

#include <cassert>

/**
 * @brief floor(sqrt(n))
 * @param n 平方根を求めたい整数 long long 型で 0 <= n < 2^63 を満たす
 * @return long long
 */
inline long long sqrtll(long long n) {
    assert(n >= 0);
    // 3037000500 = ceil(sqrt(2^63))
    long long le = 0, ri = 3037000500;
    while (ri - le > 1) {
        long long mid = (le + ri) / 2;
        if (mid * mid <= n)
            le = mid;
        else
            ri = mid;
    }
    return le;
}
