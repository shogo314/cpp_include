#pragma once
#include <algorithm>
#include <cmath>
#include <numeric>
#include <type_traits>
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

template <class C, typename T>
inline constexpr bool contains(const C &c, const T &t) {
    return c.find(t) != c.end();
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

template <class F, ENABLE_IF_T(std::is_function_v<F>)>
inline constexpr long long binary_search(long long ok, long long ng, F f) {
    assert(f(ok));
    assert(not f(ng));
    while (std::abs(ok - ng) > 1) {
        long long mid = (ok + ng) / 2;
        if (f(mid)) {
            ok = mid;
        } else {
            ng = mid;
        }
    }
    return ok;
}

template <class F, ENABLE_IF_T(std::is_function_v<F>)>
inline constexpr long double binary_search(long double ok, long double ng, F f) {
    assert(f(ok));
    assert(not f(ng));
    for (int i = 0; i < 100; i++) {
        long double mid = (ok + ng) / 2;
        if (f(mid)) {
            ok = mid;
        } else {
            ng = mid;
        }
    }
    return ok;
}
