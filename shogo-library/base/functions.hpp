#pragma once
// https://github.com/shogo314/shogo-library/blob/a2fefc7ec36d798ade24522d799381341eac20dd/src/base/functions.hpp

/**
 * @file functions.hpp
 * @brief 汎用関数 (chmin, chmax など)
 */

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>

template <typename T1, typename T2>
inline constexpr bool chmin(T1& a, T2 b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}

template <typename T1, typename T2>
inline constexpr bool chmax(T1& a, T2 b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}

inline constexpr long long max(const long long& t1, const long long& t2) {
    return std::max<long long>(t1, t2);
}

inline constexpr long long min(const long long& t1, const long long& t2) {
    return std::min<long long>(t1, t2);
}

using std::abs;
using std::gcd;
using std::lcm;
using std::size;

template <typename T>
constexpr T extgcd(const T& a, const T& b, T& x, T& y) {
    T d = a;
    if (b != 0) {
        d = extgcd(b, a % b, y, x);
        y -= (a / b) * x;
    } else {
        x = 1;
        y = 0;
    }
    return d;
}

/**
 * 0 <= x < a
 */
inline constexpr bool inrange(long long x, long long a) {
    return 0 <= x && x < a;
}
/**
 * a <= x < b
 */
inline constexpr bool inrange(long long x, long long a, long long b) {
    return a <= x && x < b;
}
/**
 * 0 <= x < a and 0 <= y < b
 */
inline constexpr bool inrect(long long x, long long y, long long a, long long b) {
    return 0 <= x && x < a && 0 <= y && y < b;
}
