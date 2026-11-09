#pragma once
// https://github.com/shogo314/shogo-library/blob/27998142f20ef4e094add3b53cc9c070db4ddbf8/src/math/modular-arithmetic.hpp

/**
 * @file modular-arithmetic.hpp
 * @brief mod 演算 (積、冪乗、逆元、離散対数、平方根)
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <unordered_map>

/**
 * @brief a * b mod m を返す (途中で __int128 を使うので m < 2^63 ならあふれない)
 */
constexpr long long mul_mod(long long a, long long b, long long m) {
    long long res = static_cast<long long>(static_cast<__int128>(a) * b % m);
    if (res < 0) res += m;
    return res;
}

/**
 * @brief a^(-1) mod m を返す
 * @note a と m は互いに素であること。O(log m)
 */
constexpr long long inv_mod(long long a, long long m) {
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
constexpr long long pow_mod(long long a, long long n, long long m) {
    assert(m >= 1);
    a %= m;
    if (a < 0) a += m;
    unsigned long long exponent = static_cast<unsigned long long>(n);
    if (n < 0) {
        exponent = 0ULL - exponent;
        a = inv_mod(a, m);
    }
    long long res = 1 % m;
    while (exponent > 0) {
        if (exponent & 1) res = mul_mod(res, a, m);
        a = mul_mod(a, a, m);
        exponent >>= 1;
    }
    return res;
}

/**
 * @brief a^x ≡ b (mod m) となる最小の x >= 0 (なければ -1)
 * @note a と m は互いに素でなくてもよい。Baby-step Giant-step で O(√m) の時間とメモリ
 */
inline long long discrete_log(long long a, long long b, long long m) {
    assert(m >= 1);
    a %= m, b %= m;
    if (a < 0) a += m;
    if (b < 0) b += m;
    if (m == 1) return 0;
    // a と m が互いに素になるまで両辺を gcd で割る。k = a^add / (割った gcd の積) mod m
    long long k = 1, add = 0, g;
    while ((g = std::gcd(a, m)) > 1) {
        if (b == k) return add;
        if (b % g != 0) return -1;
        b /= g, m /= g, add++;
        k = mul_mod(k, a / g, m);
    }
    // k a^x ≡ b を x = n p - q (1 <= p <= n、0 <= q <= n) と書いて解く
    const long long n = static_cast<long long>(std::sqrt(static_cast<long double>(m))) + 1;
    std::unordered_map<long long, long long> baby;
    baby.reserve(2 * n + 2);
    // b a^q を q の小さい順に入れ、同じ値なら大きい q で上書きする (x を小さくするため)
    for (long long q = 0, cur = b; q <= n; q++, cur = mul_mod(cur, a, m)) baby[cur] = q;
    const long long an = pow_mod(a, n, m);
    for (long long p = 1, cur = k; p <= n; p++) {
        cur = mul_mod(cur, an, m);
        auto it = baby.find(cur);
        if (it != baby.end()) return n * p - it->second + add;
    }
    return -1;
}

/**
 * @brief 素数 p について x^2 ≡ a (mod p) となる x のうち小さい方 (なければ -1)
 * @note もう一方の解は p - x。Tonelli-Shanks で O(log^2 p)
 */
inline long long sqrt_mod(long long a, long long p) {
    assert(p >= 2);
    a %= p;
    if (a < 0) a += p;
    if (a == 0 || p == 2) return a;
    // オイラーの規準: a^((p-1)/2) が 1 でなければ平方非剰余
    if (pow_mod(a, (p - 1) / 2, p) != 1) return -1;
    long long q = p - 1, s = 0;
    while (q % 2 == 0) q /= 2, s++;
    // 平方非剰余 z を探す
    long long z = 2;
    while (pow_mod(z, (p - 1) / 2, p) != p - 1) z++;
    long long c = pow_mod(z, q, p), t = pow_mod(a, q, p), r = pow_mod(a, (q + 1) / 2, p);
    // r^2 = a t を保ちながら、t の位数を下げていく
    while (t != 1) {
        long long i = 0, tt = t;
        while (tt != 1) tt = mul_mod(tt, tt, p), i++;
        long long b = c;
        for (long long j = 0; j < s - i - 1; j++) b = mul_mod(b, b, p);
        s = i;
        c = mul_mod(b, b, p);
        t = mul_mod(t, c, p);
        r = mul_mod(r, b, p);
    }
    return std::min(r, p - r);
}
