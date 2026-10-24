#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/dp/set-partition.hpp

/**
 * @file set-partition.hpp
 * @brief 集合を「使ってよい集合」に分けるときの最小個数 (要素が 16 個程度まで)
 */

#include <cassert>
#include <vector>

/**
 * @brief {0, ..., n-1} を、feasible[T] が true の集合 T いくつかに重ならないように分けるときの最小個数 (分けられなければ -1) O(3^n)
 * feasible の大きさは 2^n。dp[S] = min (dp[S \ T] + 1) を、T を「S の最小の要素を含む S の部分集合」に限って求める
 * 使ってよい集合族が部分集合について閉じているなら、重なりを許して覆うときの最小個数とも一致する
 */
inline long long min_set_partition(int n, const std::vector<bool>& feasible) {
    assert(static_cast<long long>(feasible.size()) == (1LL << n));
    const int INF = n + 1;
    std::vector<int> dp(1 << n, INF);
    dp[0] = 0;
    for (int S = 1; S < (1 << n); S++) {
        const int low = S & -S, rest = S ^ low;
        // rest の部分集合 U について T = U | low を試す
        for (int U = rest;; U = (U - 1) & rest) {
            const int T = U | low;
            if (feasible[T] && dp[S ^ T] + 1 < dp[S]) dp[S] = dp[S ^ T] + 1;
            if (U == 0) break;
        }
    }
    return dp[(1 << n) - 1] >= INF ? -1 : dp[(1 << n) - 1];
}
