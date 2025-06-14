#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <vector>

#include "traits.hpp"

template <typename T>
std::vector<std::ptrdiff_t> sorted_idx(const std::vector<T> &v, bool reverse = false) {
    std::vector<std::ptrdiff_t> ret(v.size());
    std::iota(ret.begin(), ret.end(), 0);
    std::sort(ret.begin(), ret.end(), [&](std::ptrdiff_t i, std::ptrdiff_t j) {
        return v[i] < v[j];
    });
    if (reverse) std::reverse(ret.begin(), ret.end());
    return ret;
}

template <typename T>
inline std::vector<T> &operator++(std::vector<T> &v) {
    for (auto &e : v) e++;
    return v;
}
template <typename T>
inline std::vector<T> operator++(std::vector<T> &v, int) {
    auto res = v;
    for (auto &e : v) e++;
    return res;
}
template <typename T>
inline std::vector<T> &operator--(std::vector<T> &v) {
    for (auto &e : v) e--;
    return v;
}
template <typename T>
inline std::vector<T> operator--(std::vector<T> &v, int) {
    auto res = v;
    for (auto &e : v) e--;
    return res;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> &operator+=(std::vector<T> &v1, const std::vector<U> &v2) {
    if (v2.size() > v1.size()) {
        v1.resize(v2.size());
    }
    for (size_t i = 0; i < v2.size(); i++) {
        v1[i] += v2[i];
    }
    return v1;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> operator+(const std::vector<T> &v1, const std::vector<U> &v2) {
    std::vector<T> res(v1);
    return res += v2;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> &operator+=(std::vector<T> &v, const U &u) {
    for (T &e : v) {
        e += u;
    }
    return v;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> operator+(const std::vector<T> &v, const U &u) {
    std::vector<T> res(v);
    return res += u;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> operator+(const U &u, const std::vector<T> &v) {
    std::vector<T> res(v);
    return res += u;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> &operator*=(std::vector<T> &v1, const std::vector<U> &v2) {
    if (v2.size() > v1.size()) {
        v1.resize(v2.size());
    }
    for (size_t i = 0; i < v2.size(); i++) {
        v1[i] *= v2[i];
    }
    for (size_t i = v2.size(); i < v1.size(); i++) {
        v1[i] *= U(0);
    }
    return v1;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> operator*(const std::vector<T> &v1, const std::vector<U> &v2) {
    std::vector<T> res(v1);
    return res *= v2;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> &operator*=(std::vector<T> &v, const U &u) {
    for (T &e : v) {
        e *= u;
    }
    return v;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> operator*(const std::vector<T> &v, const U &u) {
    std::vector<T> res(v);
    return res *= u;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> operator*(const U &u, const std::vector<T> &v) {
    std::vector<T> res(v);
    return res *= u;
}

template <typename T, typename U>
inline std::vector<T> &assign(std::vector<T> &v1, const std::vector<U> &v2) {
    v1.assign(v2.begin(), v2.end());
    return v1;
}

template <typename T, typename U>
inline std::vector<T> &extend(std::vector<T> &v1, const std::vector<U> &v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

template <typename T, typename U, ENABLE_IF_T(std::is_convertible_v<U, T>)>
inline std::vector<T> &operator|=(std::vector<T> &v1, const std::vector<U> &v2) {
    return extend(v1, v2);
}

template <typename T, typename U, ENABLE_IF_T(std::is_integral_v<U>)>
inline std::vector<T> &operator|=(std::vector<T> &v, const U &u) {
    std::vector<T> w(v);
    v.clear();
    for (int i = 0; i < u; i++) {
        extend(v, w);
    }
    return v;
}

template <typename T, typename U, ENABLE_IF_T(std::is_integral_v<U>)>
inline std::vector<T> operator|(const std::vector<T> &v, const U &u) {
    std::vector<T> res(v);
    return res |= u;
}

template <typename T, typename U, ENABLE_IF_T(std::is_integral_v<U>)>
inline std::vector<T> operator|(const U &u, const std::vector<T> &v) {
    std::vector<T> res(v);
    return res |= u;
}

template <typename T>
inline std::vector<T> abs(const std::vector<T> &v) {
    std::vector<T> ret;
    ret.reserve(v.size());
    for (const T &e : v) ret.push_back(std::abs(e));
    return ret;
}

template <typename T>
std::vector<T> cumulative_sum(std::vector<T> v) {
    v.insert(v.begin(), T{});
    std::vector<T> ret(v.size());
    std::partial_sum(v.begin(), v.end(), ret.begin());
    return ret;
}

template <typename T, ENABLE_IF_T(std::is_integral_v<T>)>
std::vector<T> iota(T n) {
    assert(n >= 0);
    std::vector<T> ret(n);
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

template <typename T>
std::vector<T> unique(const std::vector<T> &v) {
    std::vector<T> res(v);
    std::sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());
    return res;
}

long long radix_convert(const std::vector<long long> &v, int base = 10) {
    long long res = 0;
    for (int i = v.size() - 1; i >= 0; i--) {
        res <<= base;
        res += v[i];
    }
    return res;
}

std::vector<long long> radix_convert(long long v, int base = 10) {
    std::vector<long long> res;
    while (v > 0) {
        res.push_back(v % base);
        v /= base;
    }
    std::reverse(res.begin(), res.end());
    return res;
}
