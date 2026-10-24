#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/sqrt.hpp

/**
 * @file sqrt.hpp
 * @brief 整数の平方根・k 乗根
 */

#include <cassert>
#include <cmath>

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

/**
 * @brief floor(a^(1/k))
 *
 * 浮動小数点で近い値を求め、オーバーフローしないように掛け算で確かめて合わせる
 * @param a 0 <= a < 2^64
 * @param k 1 以上
 * @return unsigned long long
 */
inline unsigned long long kth_root(unsigned long long a, long long k) {
    assert(k >= 1);
    if (k == 1 || a <= 1) return a;
    if (k >= 64) return 1;  // a < 2^64 なので
    // x^k <= a か
    auto fits = [&](unsigned long long x) {
        unsigned long long p = 1;
        for (long long i = 0; i < k; i++) {
            if (p > a / x) return false;
            p *= x;
        }
        return true;
    };
    unsigned long long x = static_cast<unsigned long long>(std::pow(static_cast<long double>(a), 1.0L / k));
    if (x == 0) x = 1;
    while (!fits(x)) x--;
    while (fits(x + 1)) x++;
    return x;
}
