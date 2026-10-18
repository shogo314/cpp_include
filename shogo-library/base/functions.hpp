#pragma once
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/base/functions.hpp

/**
 * @file functions.hpp
 * @brief 汎用関数 (chmin, chmax など)
 */

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <numeric>
#if __cplusplus >= 202002L
#include <ranges>
#endif
#include <type_traits>
#include <utility>

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

#if __cplusplus >= 202002L
using std::midpoint;
#else
// C++20 の std::midpoint と同じ (整数は a の側に丸め、あふれない)
template <typename T>
inline constexpr std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, T> midpoint(T a, T b) noexcept {
    using U = std::make_unsigned_t<T>;
    if (a <= b) return static_cast<T>(a + static_cast<T>(static_cast<U>(static_cast<U>(b) - static_cast<U>(a)) >> 1));
    return static_cast<T>(a - static_cast<T>(static_cast<U>(static_cast<U>(a) - static_cast<U>(b)) >> 1));
}
template <typename T>
inline constexpr std::enable_if_t<std::is_floating_point_v<T>, T> midpoint(T a, T b) noexcept {
    constexpr T lo = std::numeric_limits<T>::min() * 2;
    constexpr T hi = std::numeric_limits<T>::max() / 2;
    const T abs_a = a < 0 ? -a : a, abs_b = b < 0 ? -b : b;
    if (abs_a <= hi && abs_b <= hi) return (a + b) / 2;
    if (abs_a < lo) return a + b / 2;
    if (abs_b < lo) return a / 2 + b;
    return a / 2 + b / 2;
}
template <typename T>
inline constexpr std::enable_if_t<std::is_object_v<T>, T*> midpoint(T* a, T* b) noexcept {
    return a + (b - a) / 2;
}
#endif

/**
 * 要素数を long long で返す (std::ssize の long long 版)
 */
#if __cplusplus >= 202002L
template <std::ranges::sized_range R>
inline constexpr long long len(R&& r) {
    return static_cast<long long>(std::ranges::size(r));
}
#else
template <typename C>
inline constexpr auto len(const C& c) -> decltype(static_cast<long long>(std::size(c))) {
    return static_cast<long long>(std::size(c));
}
#endif

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
 * {floor(a / b), a - b * floor(a / b)} (b != 0、負の数も可)
 * 余りは 0 または b と同じ符号になる
 */
template <typename T1, typename T2>
inline constexpr std::pair<std::common_type_t<T1, T2>, std::common_type_t<T1, T2>> divmod(T1 a, T2 b) {
    std::common_type_t<T1, T2> q = floordiv(a, b);
    return {q, a - q * b};
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
