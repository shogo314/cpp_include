#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/imos-2d.hpp

/**
 * @file imos-2d.hpp
 * @brief 2 次元のいもす法
 */

#include <cassert>
#include <vector>

/**
 * @brief 2 次元のいもす法 (長方形への加算をためておき、最後にまとめて各マスの値を求める)
 *
 * add は 4 隅に足すだけで O(1)、build で 2 方向の累積和をとって O(hw)。build の後は get で値を読む
 * 例:
 * Imos2D<int> imos(h, w);
 * imos.add(0, 0, 3, 2, 1);  // 行 [0, 3)、列 [0, 2) に 1 を足す
 * imos.build();
 * int x = imos.get(1, 1);
 *
 * @tparam T 値の型
 */
template <typename T>
class Imos2D {
    long long h, w;
    std::vector<std::vector<T>> a;  // (h + 1) × (w + 1)
    bool built = false;

   public:
    /**
     * @brief h 行 w 列で、すべて 0
     */
    Imos2D(long long h, long long w) : h(h), w(w), a(h + 1, std::vector<T>(w + 1, T(0))) {}
    /**
     * @brief 行数
     */
    long long height() const { return h; }
    /**
     * @brief 列数
     */
    long long width() const { return w; }
    /**
     * @brief 行 [i1, i2)、列 [j1, j2) のマスに x を足す O(1)。build の前に限る
     */
    void add(long long i1, long long j1, long long i2, long long j2, const T& x) {
        assert(!built);
        assert(0 <= i1 && i1 <= i2 && i2 <= h && 0 <= j1 && j1 <= j2 && j2 <= w);
        a[i1][j1] += x, a[i1][j2] -= x, a[i2][j1] -= x, a[i2][j2] += x;
    }
    /**
     * @brief 累積和をとり、各マスの値を求める O(hw)。1 回だけ呼ぶ
     */
    void build() {
        assert(!built);
        built = true;
        for (long long i = 0; i <= h; i++)
            for (long long j = 1; j <= w; j++) a[i][j] += a[i][j - 1];
        for (long long i = 1; i <= h; i++)
            for (long long j = 0; j <= w; j++) a[i][j] += a[i - 1][j];
    }
    /**
     * @brief マス (i, j) の値 O(1)。build の後に限る
     */
    T get(long long i, long long j) const {
        assert(built);
        assert(0 <= i && i < h && 0 <= j && j < w);
        return a[i][j];
    }
};
