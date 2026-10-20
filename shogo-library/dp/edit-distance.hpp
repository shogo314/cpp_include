#pragma once
// https://github.com/shogo314/shogo-library/blob/93de89301cdeac655ce53761092d9556752fd185/src/dp/edit-distance.hpp

/**
 * @file edit-distance.hpp
 * @brief 編集距離 (レーベンシュタイン距離)
 */

#include <algorithm>
#include <iterator>
#include <vector>

/**
 * @brief s を t に変えるのに必要な挿入・削除・置換の最小回数 O(nm)
 * 要素は == で比べる。メモリは O(m)
 */
template <typename R1, typename R2>
long long edit_distance(const R1& s, const R2& t) {
    auto a = std::begin(s);
    auto b = std::begin(t);
    long long n = std::end(s) - a, m = std::end(t) - b;
    // dp[j] は s[0, i) と t[0, j) の編集距離
    std::vector<long long> dp(m + 1);
    for (long long j = 0; j <= m; j++) dp[j] = j;
    for (long long i = 1; i <= n; i++) {
        long long diag = dp[0];  // s[0, i - 1) と t[0, j - 1) の編集距離
        dp[0] = i;
        for (long long j = 1; j <= m; j++) {
            long long up = dp[j];
            dp[j] = std::min({dp[j] + 1, dp[j - 1] + 1, diag + (a[i - 1] == b[j - 1] ? 0 : 1)});
            diag = up;
        }
    }
    return dp[m];
}
