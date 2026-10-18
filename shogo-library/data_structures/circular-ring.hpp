#pragma once
// https://github.com/shogo314/shogo-library/blob/b591e1ab6db09d0025e385b1bb5f4d8982cc4d1f/src/data_structures/circular-ring.hpp

/**
 * @file circular-ring.hpp
 * @brief 円環
 */

#include <cassert>
#include <cstddef>
#include <vector>

/**
 * @brief 円環
 * @tparam T
 */
template <typename T>
struct CircularRing {
    std::vector<T> data;
    explicit CircularRing(const std::vector<T>& v) : data(v) {}
    CircularRing(const CircularRing&) = default;
    CircularRing() = default;
    explicit CircularRing(long long s) : data(s) {}
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
