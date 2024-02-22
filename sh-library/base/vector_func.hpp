#pragma once

#include <vector>

template <typename T>
inline T max(const std::vector<T>& v) {
    return *std::max_element(v.begin(), v.end());
}

template <typename T>
inline T min(const std::vector<T>& v) {
    return *std::min_element(v.begin(), v.end());
}

template <typename T>
std::vector<T> sorted(std::vector<T> v) {
    std::sort(v.begin(), v.end());
    return v;
}

template <typename T>
std::vector<T> reversed(std::vector<T> v) {
    std::reverse(v.begin(), v.end());
    return v;
}

template <typename T>
inline T sum(const std::vector<T>& v, T init = T{}) {
    return std::accumulate(v.begin(), v.end(), init);
}

template <typename T>
inline T product(const std::vector<T>& v, T init = T{1}) {
    for (size_t i = 0; i < v.size(); i++) {
        init = init * v[i];
    }
    return init;
}

template <typename T>
inline T average(const std::vector<T>& v) {
    assert(not v.empty());
    return sum(v) / v.size();
}

template <typename T>
inline T median(const std::vector<T>& v) {
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
inline size_t index(const std::vector<T>& v, const T& x) {
    return std::distance(v.begin(), std::find(v.begin(), v.end(), x));
}
