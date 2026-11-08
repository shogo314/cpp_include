#pragma once

/**
 * @file quotients.hpp
 * @brief 商の列挙 (floor(n / i)・floor((a x + b) / m) が一定の区間)
 */

#include <cassert>
#include <limits>
#include <vector>

#include "../base/functions.hpp"
// https://github.com/shogo314/shogo-library/blob/77ebb090933789a5c7f4dc5561b48ac9861e1e0b/src/math/quotients.hpp

/**
 * @brief 商 q と、それをとる添字の区間 [l, r)
 */
struct QuotientRange {
    long long q;  //!< 商
    long long l;  //!< 区間の左端 (含む)
    long long r;  //!< 区間の右端 (含まない)
};

/**
 * @brief i = 1, ..., n を floor(n / i) の値で区間に分け、i の昇順 (商の降順) に f(q, l, r) を呼ぶ O(sqrt(n))
 * l <= i < r なら floor(n / i) == q
 * @pre 0 <= n < std::numeric_limits<long long>::max()
 */
template <typename F>
void for_each_quotient(long long n, F f) {
    assert(0 <= n && n < std::numeric_limits<long long>::max());
    for (long long l = 1; l <= n;) {
        long long q = n / l, r = n / q + 1;
        f(q, l, r);
        l = r;
    }
}

/**
 * @brief i = 1, ..., n を floor(n / i) の値で区間に分けたもの (i の昇順、つまり商の降順) O(sqrt(n))
 * 区間の個数は 2 sqrt(n) 以下
 */
inline std::vector<QuotientRange> enumerate_quotients(long long n) {
    std::vector<QuotientRange> res;
    for_each_quotient(n, [&](long long q, long long l, long long r) { res.push_back({q, l, r}); });
    return res;
}

/**
 * @brief x = 0, ..., n - 1 を floor((a x + b) / m) の値で区間に分け、x の昇順に f(q, l, r) を呼ぶ O(区間数)
 * @pre n >= 0、m >= 1、各商が long long に収まること。a, b は負でもよい
 */
template <typename F>
void for_each_quotient_linear(long long n, long long m, long long a, long long b, F f) {
    assert(n >= 0 && m >= 1);
    const __int128_t slope = a;
    for (long long l = 0; l < n;) {
        auto [q, rem] = divmod(slope * l + b, static_cast<__int128_t>(m));
        assert(std::numeric_limits<long long>::min() <= q && q <= std::numeric_limits<long long>::max());
        __int128_t length = n - l;
        // 正の傾きでは次の商に届くまで、負の傾きでは余りが負になるまで進む
        if (a > 0) chmin(length, ceildiv(m - rem, slope));
        if (a < 0) chmin(length, rem / (-slope) + 1);
        const long long r = l + static_cast<long long>(length);
        f(static_cast<long long>(q), l, r);
        l = r;
    }
}

/**
 * @brief x = 0, ..., n - 1 を floor((a x + b) / m) の値で区間に分けたもの (x の昇順) O(区間数)
 * @pre n >= 0、m >= 1、各商が long long に収まること。a, b は負でもよい
 */
inline std::vector<QuotientRange> enumerate_quotients_linear(long long n, long long m, long long a, long long b) {
    std::vector<QuotientRange> res;
    for_each_quotient_linear(n, m, a, b, [&](long long q, long long l, long long r) { res.push_back({q, l, r}); });
    return res;
}
