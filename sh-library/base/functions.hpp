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

template <typename M, typename N, class F, ENABLE_IF_T(std::is_integral_v<M> and std::is_integral_v<N> and is_lambda_func_v<F, std::common_type_t<M, N>>)>
inline constexpr std::common_type_t<M, N> binary_search(M ok, N ng, F f) {
    std::common_type_t<M, N> _ok = ok, _ng = ng;
    assert(f(_ok));
    while (std::abs(ok - ng) > 1) {
        std::common_type_t<M, N> mid = (ok + ng) / 2;
        if (f(mid)) {
            ok = mid;
        } else {
            ng = mid;
        }
    }
    return ok;
}

template <typename M, typename N, class F, ENABLE_IF_T(not std::is_integral_v<M> and not std::is_integral_v<N> and is_lambda_func_v<F, std::common_type_t<M, N>>)>
inline constexpr std::common_type_t<M, N> binary_search(M ok, N ng, F f) {
    std::common_type_t<M, N> _ok = ok, _ng = ng;
    assert(f(_ok));
    for (int i = 0; i < 100; i++) {
        std::common_type_t<M, N> mid = (ok + ng) / 2;
        if (f(mid)) {
            ok = mid;
        } else {
            ng = mid;
        }
    }
    return ok;
}
