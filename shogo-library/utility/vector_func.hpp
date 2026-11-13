#pragma once

/**
 * @file vector_func.hpp
 * @brief vector に対する演算子と関数
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <numeric>
#include <type_traits>
#include <vector>

#include "traits.hpp"
// https://github.com/shogo314/shogo-library/blob/9a3850ecb2614059a50786efbc668c34530bbd74/src/utility/vector_func.hpp

/**
 * @brief 値を proj で写して comp の順に安定ソートしたときの添字の並び (APL の grade up)
 */
template <typename T, typename F = std::less<>, typename P = internal_traits::identity>
std::vector<long long> argsort(const std::vector<T>& v, F comp = F(), P proj = P()) {
    std::vector<long long> ret(v.size());
    std::iota(ret.begin(), ret.end(), 0LL);
    std::stable_sort(ret.begin(), ret.end(), [&](long long i, long long j) {
        return std::invoke(comp, std::invoke(proj, v[i]), std::invoke(proj, v[j]));
    });
    return ret;
}

/**
 * @brief 各要素の順位 (proj で写して comp の順で v[i] より前に来る要素の個数)。同率は同じ順位で、次の順位は飛ばす
 */
template <typename T, typename F = std::less<>, typename P = internal_traits::identity>
std::vector<long long> rank(const std::vector<T>& v, F comp = F(), P proj = P()) {
    const long long n = v.size();
    std::vector<long long> p = argsort(v, comp, proj), ret(n);
    for (long long i = 0; i < n; i++) {
        ret[p[i]] = i > 0 && !std::invoke(comp, std::invoke(proj, v[p[i - 1]]), std::invoke(proj, v[p[i]])) ? ret[p[i - 1]] : i;
    }
    return ret;
}

/**
 * @brief 逆置換 (ret[p[i]] = i)。p は 0, ..., n-1 の置換であること
 */
#if __cplusplus >= 202002L
template <typename I>
    requires(std::integral<I>)
#else
template <typename I, ENABLE_IF_T(std::is_integral_v<I>)>
#endif
std::vector<long long> inverse_permutation(const std::vector<I>& p) {
    const long long n = p.size();
    std::vector<long long> ret(n, -1);
    for (long long i = 0; i < n; i++) {
        const long long x = static_cast<long long>(p[i]);
        assert(0 <= x && x < n && ret[x] == -1);
        ret[x] = i;
    }
    return ret;
}

/**
 * @brief {v[idx[0]], v[idx[1]], ...} (Uiua の select)
 */
#if __cplusplus >= 202002L
template <typename T, typename I>
    requires(std::integral<I>)
#else
template <typename T, typename I, ENABLE_IF_T(std::is_integral_v<I>)>
#endif
std::vector<T> select(const std::vector<T>& v, const std::vector<I>& idx) {
    std::vector<T> ret;
    ret.reserve(idx.size());
    for (I i : idx) {
        assert(0 <= static_cast<long long>(i) && static_cast<long long>(i) < static_cast<long long>(v.size()));
        ret.push_back(v[i]);
    }
    return ret;
}

template <typename T>
inline std::vector<T>& operator++(std::vector<T>& v) {
    for (auto& e : v) e++;
    return v;
}
template <typename T>
inline std::vector<T> operator++(std::vector<T>& v, int) {
    auto res = v;
    for (auto& e : v) e++;
    return res;
}
template <typename T>
inline std::vector<T>& operator--(std::vector<T>& v) {
    for (auto& e : v) e--;
    return v;
}
template <typename T>
inline std::vector<T> operator--(std::vector<T>& v, int) {
    auto res = v;
    for (auto& e : v) e--;
    return res;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T>& operator+=(std::vector<T>& v1, const std::vector<U>& v2) {
    if (v2.size() > v1.size()) {
        v1.resize(v2.size());
    }
    for (std::size_t i = 0; i < v2.size(); i++) {
        v1[i] += v2[i];
    }
    return v1;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T> operator+(const std::vector<T>& v1, const std::vector<U>& v2) {
    std::vector<T> res(v1);
    return res += v2;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T>& operator+=(std::vector<T>& v, const U& u) {
    for (T& e : v) {
        e += u;
    }
    return v;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T> operator+(const std::vector<T>& v, const U& u) {
    std::vector<T> res(v);
    return res += u;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T> operator+(const U& u, const std::vector<T>& v) {
    std::vector<T> res(v);
    return res += u;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T>& operator*=(std::vector<T>& v1, const std::vector<U>& v2) {
    if (v2.size() > v1.size()) {
        v1.resize(v2.size());
    }
    for (std::size_t i = 0; i < v2.size(); i++) {
        v1[i] *= v2[i];
    }
    for (std::size_t i = v2.size(); i < v1.size(); i++) {
        v1[i] *= U(0);
    }
    return v1;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T> operator*(const std::vector<T>& v1, const std::vector<U>& v2) {
    std::vector<T> res(v1);
    return res *= v2;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T>& operator*=(std::vector<T>& v, const U& u) {
    for (T& e : v) {
        e *= u;
    }
    return v;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T> operator*(const std::vector<T>& v, const U& u) {
    std::vector<T> res(v);
    return res *= u;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T> operator*(const U& u, const std::vector<T>& v) {
    std::vector<T> res(v);
    return res *= u;
}

template <typename T, typename U>
inline std::vector<T>& assign(std::vector<T>& v1, const std::vector<U>& v2) {
    v1.assign(v2.begin(), v2.end());
    return v1;
}

template <typename T, typename U>
inline std::vector<T>& extend(std::vector<T>& v1, const std::vector<U>& v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::convertible_to<U, T>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
#endif
inline std::vector<T>& operator|=(std::vector<T>& v1, const std::vector<U>& v2) {
    return extend(v1, v2);
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::integral<U>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_integral_v<U>)>
#endif
inline std::vector<T>& operator|=(std::vector<T>& v, const U& u) {
    if constexpr (std::is_signed_v<U>) assert(u >= 0);
    std::vector<T> w(v);
    v.clear();
    for (long long i = 0; i < static_cast<long long>(u); i++) {
        extend(v, w);
    }
    return v;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::integral<U>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_integral_v<U>)>
#endif
inline std::vector<T> operator|(const std::vector<T>& v, const U& u) {
    std::vector<T> res(v);
    return res |= u;
}

#if __cplusplus >= 202002L
template <typename T, typename U>
    requires(std::integral<U>)
#else
template <typename T, typename U, ENABLE_IF_T(std::is_integral_v<U>)>
#endif
inline std::vector<T> operator|(const U& u, const std::vector<T>& v) {
    std::vector<T> res(v);
    return res |= u;
}

template <typename T>
inline std::vector<T> abs(const std::vector<T>& v) {
    std::vector<T> ret;
    ret.reserve(v.size());
    for (const T& e : v) ret.push_back(std::abs(e));
    return ret;
}

template <typename T>
std::vector<T> cumulative_sum(std::vector<T> v) {
    v.insert(v.begin(), T{});
    std::vector<T> ret(v.size());
    std::partial_sum(v.begin(), v.end(), ret.begin());
    return ret;
}

#if __cplusplus >= 202002L
template <typename T>
    requires(std::integral<T>)
#else
template <typename T, ENABLE_IF_T(std::is_integral_v<T>)>
#endif
std::vector<T> iota(T n) {
    assert(n >= 0);
    std::vector<T> ret(n);
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

template <typename T>
std::vector<T> unique(const std::vector<T>& v) {
    std::vector<T> res(v);
    std::sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());
    return res;
}

inline long long radix_convert(const std::vector<long long>& v, int base = 10) {
    assert(base >= 2);
    long long res = 0;
    for (long long d : v) {
        assert(0 <= d && d < base);
        res = res * base + d;
    }
    return res;
}

inline std::vector<long long> radix_convert(long long v, int base = 10) {
    assert(v >= 0 && base >= 2);
    std::vector<long long> res;
    while (v > 0) {
        res.push_back(v % base);
        v /= base;
    }
    std::reverse(res.begin(), res.end());
    return res;
}
