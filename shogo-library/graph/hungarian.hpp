#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/graph/hungarian.hpp

/**
 * @file hungarian.hpp
 * @brief 割当問題 (ハンガリアン法)
 */

#include <cassert>
#include <limits>
#include <vector>

/**
 * @brief hungarian の戻り値
 */
template <typename T>
struct Assignment {
    T cost;                        //!< コストの総和
    std::vector<long long> match;  //!< match[i]: 行 i に割り当てた列
};

/**
 * @brief n × m (n <= m) のコスト行列 a で、各行に異なる列を 1 つずつ割り当てるときのコストの総和の最小値 O(n^2 m)
 *
 * 最大化したいときは、符号を反転したコストで呼ぶ
 * @param a コスト行列 (負でもよい)。行の長さはそろっていること
 */
template <typename T>
Assignment<T> hungarian(const std::vector<std::vector<T>>& a) {
    const long long n = a.size();
    if (n == 0) return {T{}, {}};
    const long long m = a[0].size();
    assert(n <= m);
    for (const auto& row : a) assert(static_cast<long long>(row.size()) == m);
    const T inf = std::numeric_limits<T>::max() / 4;
    // 1 始まりで扱い、列 0 を「まだ割り当てていない行」の置き場にする
    // u・v は行と列のポテンシャル、p[j] は列 j に割り当てた行、way[j] は増加路で列 j の 1 つ前の列
    std::vector<T> u(n + 1), v(m + 1);
    std::vector<long long> p(m + 1), way(m + 1);
    for (long long i = 1; i <= n; i++) {
        p[0] = i;
        long long j0 = 0;
        std::vector<T> minv(m + 1, inf);
        std::vector<bool> used(m + 1, false);
        // 行 i から、まだ割り当てていない列に着くまで、被約コストが最小の列を 1 つずつ足していく
        do {
            used[j0] = true;
            const long long i0 = p[j0];
            T delta = inf;
            long long j1 = 0;
            for (long long j = 1; j <= m; j++) {
                if (used[j]) continue;
                T cur = a[i0 - 1][j - 1] - u[i0] - v[j];
                if (cur < minv[j]) minv[j] = cur, way[j] = j0;
                if (minv[j] < delta) delta = minv[j], j1 = j;
            }
            for (long long j = 0; j <= m; j++) {
                if (used[j]) {
                    u[p[j]] += delta;
                    v[j] -= delta;
                } else {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);
        // 増加路に沿って割り当てを入れ替える
        do {
            long long j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);
    }
    Assignment<T> res{T{}, std::vector<long long>(n, -1)};
    for (long long j = 1; j <= m; j++) {
        if (p[j] != 0) res.match[p[j] - 1] = j - 1;
    }
    for (long long i = 0; i < n; i++) res.cost += a[i][res.match[i]];
    return res;
}
