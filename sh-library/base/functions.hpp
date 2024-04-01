#pragma once
#include <algorithm>
#include <cmath>
#include <numeric>
#include "traits.hpp"

template <typename T1, typename T2>
inline constexpr bool chmin(T1 &a, T2 b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}

template <typename T1, typename T2>
inline constexpr bool chmax(T1 &a, T2 b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}

inline constexpr long long max(const long long &t1, const long long &t2) {
    return std::max<long long>(t1, t2);
}

inline constexpr long long min(const long long &t1, const long long &t2) {
    return std::min<long long>(t1, t2);
}

using std::abs;
using std::gcd;
using std::lcm;
using std::size;

template <typename T>
constexpr T extgcd(const T &a, const T &b, T &x, T &y) {
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

template <typename M, typename N, class F, ENABLE_IF_T(std::is_integral_v<std::common_type_t<M, N>> and std::is_invocable_r_v<bool, F, std::common_type_t<M, N>>)>
inline constexpr std::common_type_t<M, N> binary_search(const M &ok, const N &ng, F f) {
    std::common_type_t<M, N> _ok = ok, _ng = ng;
    assert(f(_ok));
    while (std::abs(_ok - _ng) > 1) {
        std::common_type_t<M, N> mid = (_ok + _ng) / 2;
        if (f(mid)) {
            _ok = mid;
        } else {
            _ng = mid;
        }
    }
    return _ok;
}

template <typename M, typename N, class F, ENABLE_IF_T(not std::is_integral_v<std::common_type_t<M, N>> and std::is_invocable_r_v<bool, F, std::common_type_t<M, N>>)>
inline constexpr std::common_type_t<M, N> binary_search(const M &ok, const N &ng, F f) {
    std::common_type_t<M, N> _ok = ok, _ng = ng;
    assert(f(_ok));
    for (int i = 0; i < 100; i++) {
        std::common_type_t<M, N> mid = (_ok + _ng) / 2;
        if (f(mid)) {
            _ok = mid;
        } else {
            _ng = mid;
        }
    }
    return _ok;
}

/**
 * 0 <= x < a
 */
inline constexpr bool inrange(long long x, long long a) {
    return 0 <= x and x < a;
}
/**
 * a <= x < b
 */
inline constexpr bool inrange(long long x, long long a, long long b) {
    return a <= x and x < b;
}
/**
 * 0 <= x < a and 0 <= y < b
 */
inline constexpr bool inrect(long long x, long long y, long long a, long long b) {
    return 0 <= x and x < a and 0 <= y and y < b;
}
