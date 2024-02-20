#include <bits/stdc++.h>

template <typename T1, typename T2>
inline bool chmin(T1& a, T2 b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}

template <typename T1, typename T2>
inline bool chmax(T1& a, T2 b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}

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
    std::sort(v.begin(), v.end()) return v;
}

template <typename T>
inline T sum(const std::vector<T>& v) {
    return std::accumulate(v.begin(), v.end(), T{});
}

template <typename T>
inline T prod(const std::vector<T>& v, T init = T{1}) {
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
    return (v[u[v.size() / 2]] + v[u[(v.size() - 1) / 2]]) / 2;
}