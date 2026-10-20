#pragma once
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/data_structures/tadpole-array.hpp

/**
 * @file tadpole-array.hpp
 * @brief おたまじゃくし配列
 */

#include <cassert>
#include <cstddef>
#include <vector>
/**
 * @brief おたまじゃくし配列
 * 特定の位置でループする構造
 * @tparam T
 */
template <typename T>
class TadpoleArray {
   private:
    std::vector<T> data;
    long long k;

   public:
    /**
     * @brief 最後の次のインデックスがi
     */
    TadpoleArray(const std::vector<T>& v, long long i) : data(v) {
        assert(0 <= i && i < static_cast<long long>(data.size()));
        k = i;
    }
    TadpoleArray() = default;
    TadpoleArray(const TadpoleArray&) = default;
    /**
     * @brief i 番目の要素 (i は配列の長さを超えてもよい)
     */
    T at(long long i) const {
        assert(i >= 0);
        if (i < k) return data[i];
        return data[(i - k) % (static_cast<long long>(data.size()) - k) + k];
    }
    T operator[](long long i) const {
        return at(i);
    }
};
