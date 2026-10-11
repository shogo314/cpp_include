#pragma once
#include <cassert>
#include <vector>

/**
 * @brief 円環
 * @tparam T 
 */
template <typename T>
struct CircularRing {
    std::vector<T> data;
    CircularRing(const std::vector<T>& v) : data(v) {}
    CircularRing(const CircularRing&) = default;
    CircularRing() = default;
    CircularRing(size_t s) : data(s) {}
    T at(long long i) const {
        assert(!data.empty());
        i %= static_cast<long long>(data.size());
        if (i < 0) i += static_cast<long long>(data.size());
        return data[i];
    }
    T& at(long long i) {
        assert(!data.empty());
        i %= static_cast<long long>(data.size());
        if (i < 0) i += static_cast<long long>(data.size());
        return data[i];
    }
    T operator[](long long i) const {
        return at(i);
    }
    T& operator[](long long i) {
        return at(i);
    }
};
