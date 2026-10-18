#pragma once
// https://github.com/shogo314/shogo-library/blob/db9a2f114afe8539fdb02e5a44fc654c2297c5fa/src/base/functions.hpp

/**
 * @file functions.hpp
 * @brief 汎用関数 (chmin, chmax など)
 */

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <type_traits>

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
 * floor(a / b) (b != 0、負の数も可)
 */
template <typename T1, typename T2>
inline constexpr std::common_type_t<T1, T2> floordiv(T1 a, T2 b) {
    return a / b - (a % b != 0 && (a < 0) != (b < 0));
}
/**
 * ceil(a / b) (b != 0、負の数も可)
 */
template <typename T1, typename T2>
inline constexpr std::common_type_t<T1, T2> ceildiv(T1 a, T2 b) {
    return a / b + (a % b != 0 && (a < 0) == (b < 0));
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
