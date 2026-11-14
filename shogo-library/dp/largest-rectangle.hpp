#pragma once

/**
 * @file largest-rectangle.hpp
 * @brief ヒストグラム・グリッドの最大長方形と最大正方形
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "../utility/range.hpp"
// https://github.com/shogo314/shogo-library/blob/adb06bae4b0e202b3a1fe53ef9f9a1b30077d8cc/src/dp/largest-rectangle.hpp

/**
 * @brief ヒストグラム (幅 1 の棒が並んだもの) に含まれる長方形の面積の最大値 O(n)
 *
 * 高さが単調増加になるようにスタックを持ち、棒が取り除かれるときにその棒の高さの長方形の幅が決まる
 * @param h 棒の高さ (0 以上)
 * @return long long 面積の最大値 (h が空なら 0)
 */
template <typename T>
long long largest_rectangle_in_histogram(const std::vector<T>& h) {
    const long long n = h.size();
    long long res = 0;
    std::vector<std::pair<long long, T>> st;  // (左端, 高さ)
    for (long long i = 0; i <= n; i++) {
        T x = i < n ? h[i] : T(0);
        assert(x >= 0);
        long long left = i;
        while (!st.empty() && st.back().second >= x) {
            res = std::max(res, static_cast<long long>(st.back().second) * (i - st.back().first));
            left = st.back().first;
            st.pop_back();
        }
        st.emplace_back(left, x);
    }
    return res;
}

/**
 * @brief grid の行・列の範囲で、f(i, j) が真のマスだけからなる長方形のマスの個数の最大値 O(hw)
 *
 * 各行を底辺として、上に続く真のマスの個数をヒストグラムにする
 * @param f マス (i, j) を使えるか
 */
template <typename F>
long long largest_rectangle(const Grid& grid, const F& f) {
    std::vector<long long> height(grid.cols.size(), 0);
    long long res = 0;
    for (long long i : grid.rows) {
        for (long long j : grid.cols) {
            long long k = grid.cols.index(j);
            height[k] = f(i, j) ? height[k] + 1 : 0;
        }
        res = std::max(res, largest_rectangle_in_histogram(height));
    }
    return res;
}

/**
 * @brief grid の行・列の範囲で、f(i, j) が真のマスだけからなる正方形の辺の長さの最大値 O(hw)
 *
 * dp[i][j] = (i, j) を右下とする正方形の辺の長さの最大値 = min(上, 左, 左上) + 1
 * @param f マス (i, j) を使えるか
 */
template <typename F>
long long largest_square(const Grid& grid, const F& f) {
    std::vector<long long> dp(grid.cols.size() + 1, 0), prev(grid.cols.size() + 1, 0);
    long long res = 0;
    for (long long i : grid.rows) {
        for (long long j : grid.cols) {
            long long k = grid.cols.index(j);
            dp[k + 1] = f(i, j) ? std::min({prev[k + 1], dp[k], prev[k]}) + 1 : 0;
            res = std::max(res, dp[k + 1]);
        }
        std::swap(dp, prev);
    }
    return res;
}
