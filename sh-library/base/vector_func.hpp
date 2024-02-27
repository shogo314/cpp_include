#pragma once

#include <cmath>
#include <numeric>
#include <vector>

#include "type_traits"

template <typename T>
inline T max(const std::vector<T> &v) {
    return *std::max_element(v.begin(), v.end());
}

template <typename T>
inline T min(const std::vector<T> &v) {
    return *std::min_element(v.begin(), v.end());
}

template <typename T>
inline std::vector<T> sorted(const std::vector<T> &v) {
    std::sort(v.begin(), v.end());
    return v;
}

template <typename T>
inline std::vector<T> reversed(const std::vector<T> &v) {
    std::reverse(v.begin(), v.end());
    return v;
}

template <typename T>
inline T sum(const std::vector<T> &v, T init = {}) {
    return std::accumulate(v.begin(), v.end(), init);
}

template <typename T>
inline T product(const std::vector<T> &v, T init = {1}) {
    for (size_t i = 0; i < v.size(); i++) {
        init = init * v[i];
    }
    return init;
}

template <typename T>
T gcd(const std::vector<T> &v, T init = {0}) {
    for (const T &e : v) init = std::gcd(init, e);
    return init;
}

template <typename T>
inline T average(const std::vector<T> &v) {
    assert(not v.empty());
    return sum(v) / v.size();
}

template <typename T>
inline T median(const std::vector<T> &v) {
    assert(not v.empty());
    std::vector<size_t> u(v.size());
    std::iota(u.begin(), u.end(), 0);
    std::sort(u.begin(), u.end(), [&](size_t a, size_t b) {
        return v[a] < v[b];
    });
    if (v.size() & 1) {
        return v[u[v.size() / 2]];
    }
    return (v[u[v.size() / 2]] + v[u[v.size() / 2 - 1]]) / 2;
}

template <typename T>
inline size_t index(const std::vector<T> &v, const T &x) {
    return std::distance(v.begin(), std::find(v.begin(), v.end(), x));
}

template <typename T>
std::vector<T> &operator++(std::vector<T> &v) {
    for (auto &e : v) e++;
    return v;
}
template <typename T>
std::vector<T> operator++(std::vector<T> &v, int) {
    auto res = v;
    for (auto &e : v) e++;
    return res;
}
template <typename T>
std::vector<T> &operator--(std::vector<T> &v) {
    for (auto &e : v) e--;
    return v;
}
template <typename T>
std::vector<T> operator--(std::vector<T> &v, int) {
    auto res = v;
    for (auto &e : v) e--;
    return res;
}

template <typename T, typename U, std::enable_if_t<is_addible_with_v<T, U>, std::nullptr_t> = nullptr>
std::vector<T> &operator+=(std::vector<T> &v1, const std::vector<U> &v2) {
    if (v2.size() > v1.size()) {
        v1.resize(v2.size());
    }
    for (size_t i = 0; i < v2.size(); i++) {
        v1[i] += v2[i];
    }
    return v1;
}

template <typename T, typename U, std::enable_if_t<is_addible_with_v<T, U>, std::nullptr_t> = nullptr>
std::vector<T> &operator+=(std::vector<T> &v, const U &u) {
    for (T &e : v) {
        e += u;
    }
    return v;
}

template <typename T, typename U>
std::vector<T> &assign(std::vector<T> &v1, const std::vector<U> &v2) {
    v1.assign(v2.begin(), v2.end());
    return v1;
}

template <typename T, typename U>
std::vector<T> &extend(std::vector<T> &v1, const std::vector<U> &v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

template <typename T>
std::vector<T> abs(const std::vector<T> &v) {
    std::vector<T> ret;
    ret.reserve(v.size());
    for (const T &e : v) ret.push_back(std::abs(e));
    return ret;
}
