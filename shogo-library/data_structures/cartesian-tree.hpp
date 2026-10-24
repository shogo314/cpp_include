#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/cartesian-tree.hpp

/**
 * @file cartesian-tree.hpp
 * @brief Cartesian 木
 */

#include <functional>
#include <vector>

/**
 * @brief cartesian_tree の戻り値
 */
struct CartesianTree {
    long long root;                 //!< 根 (列が空なら -1)
    std::vector<long long> parent;  //!< 親 (根は -1)
    std::vector<long long> left;    //!< 左の子 (なければ -1)
    std::vector<long long> right;   //!< 右の子 (なければ -1)
};

/**
 * @brief 列 a の Cartesian 木 (最小の要素を根とし、その左と右の部分列の Cartesian 木をそれぞれ左と右の子にした二分木) O(n)
 *
 * 中間順に並べると元の列になる。区間 [l, r) の最小値の位置は、l と r - 1 の LCA
 * 同じ値があるときは、左にあるものを先祖にする
 * @param comp 比較関数。std::greater<T>() を渡すと最大の要素が根になる
 */
template <typename T, typename Compare = std::less<T>>
CartesianTree cartesian_tree(const std::vector<T>& a, Compare comp = Compare()) {
    const long long n = a.size();
    CartesianTree res{-1, std::vector<long long>(n, -1), std::vector<long long>(n, -1), std::vector<long long>(n, -1)};
    // 右端から根への道 (根が先頭)
    std::vector<long long> stk;
    for (long long i = 0; i < n; i++) {
        long long last = -1;
        while (!stk.empty() && comp(a[i], a[stk.back()])) {
            last = stk.back();
            stk.pop_back();
        }
        if (last != -1) res.parent[last] = i, res.left[i] = last;
        if (!stk.empty()) res.parent[i] = stk.back(), res.right[stk.back()] = i;
        stk.push_back(i);
    }
    if (n > 0) res.root = stk.front();
    return res;
}
