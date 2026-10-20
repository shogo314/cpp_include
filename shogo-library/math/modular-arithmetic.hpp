#pragma once
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/math/modular-arithmetic.hpp

/**
 * @file modular-arithmetic.hpp
 * @brief mod 演算 (積、冪乗、逆元)
 */

#include <cassert>

/**
 * @brief a * b mod m を返す (途中で __int128 を使うので m < 2^63 ならあふれない)
 */
constexpr long long modmul(long long a, long long b, long long m) {
    long long res = static_cast<long long>(static_cast<__int128>(a) * b % m);
    if (res < 0) res += m;
    return res;
}

/**
 * @brief a^(-1) mod m を返す
 * @note a と m は互いに素であること。O(log m)
 */
constexpr long long modinv(long long a, long long m) {
    assert(m >= 1);
    a %= m;
    if (a < 0) a += m;
    long long b = m, u = 1, v = 0;
    while (b) {
        // std::swap は C++17 では constexpr でないので、使わずに書く
        long long t = a / b;
        long long c = a - t * b;
        a = b, b = c;
        long long w = u - t * v;
        u = v, v = w;
    }
    assert(a == 1 && "a and m must be coprime");
    u %= m;
    if (u < 0) u += m;
    return u;
}

/**
 * @brief a^n mod m を返す
 * @note n が負なら a^(-1) の -n 乗 (a と m は互いに素であること)。O(log |n|)
 */
constexpr long long modpow(long long a, long long n, long long m) {
    assert(m >= 1);
    a %= m;
    if (a < 0) a += m;
    if (n < 0) {
        n = -n;
        a = modinv(a, m);
    }
    long long res = 1 % m;
    while (n > 0) {
        if (n & 1) res = modmul(res, a, m);
        a = modmul(a, a, m);
        n >>= 1;
    }
    return res;
}
