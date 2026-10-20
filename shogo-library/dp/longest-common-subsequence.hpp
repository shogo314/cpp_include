#pragma once
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/dp/longest-common-subsequence.hpp

/**
 * @file longest-common-subsequence.hpp
 * @brief 最長共通部分列 (LCS)
 */

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

/**
 * @brief s と t の最長共通部分列の長さ O(nm)
 * 要素は == で比べる。メモリは O(m)
 */
template <typename R1, typename R2>
long long longest_common_subsequence(const R1& s, const R2& t) {
    auto a = std::begin(s);
    auto b = std::begin(t);
    long long n = std::end(s) - a, m = std::end(t) - b;
    // dp[j] は s[0, i) と t[0, j) の LCS の長さ
    std::vector<long long> dp(m + 1);
    for (long long i = 1; i <= n; i++) {
        long long diag = 0;  // s[0, i - 1) と t[0, j - 1) の LCS の長さ
        for (long long j = 1; j <= m; j++) {
            long long up = dp[j];
            dp[j] = a[i - 1] == b[j - 1] ? diag + 1 : std::max(dp[j], dp[j - 1]);
            diag = up;
        }
    }
    return dp[m];
}

/**
 * @brief s と t の最長共通部分列を 1 つ求める O(nm)
 * s[i] == t[j] として選んだ添字の組 (i, j) を、昇順に並べたリストを返す。メモリは O(nm)
 */
template <typename R1, typename R2>
std::vector<std::pair<long long, long long>> longest_common_subsequence_restore(const R1& s, const R2& t) {
    auto a = std::begin(s);
    auto b = std::begin(t);
    long long n = std::end(s) - a, m = std::end(t) - b;
    // dp[i][j] は s[0, i) と t[0, j) の LCS の長さ
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1));
    for (long long i = 1; i <= n; i++) {
        for (long long j = 1; j <= m; j++) {
            dp[i][j] = a[i - 1] == b[j - 1] ? dp[i - 1][j - 1] + 1 : std::max(dp[i - 1][j], dp[i][j - 1]);
        }
    }
    // 後ろからたどる
    std::vector<std::pair<long long, long long>> res;
    for (long long i = n, j = m; i > 0 && j > 0;) {
        if (a[i - 1] == b[j - 1]) {
            res.emplace_back(i - 1, j - 1);
            i--, j--;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            i--;
        } else {
            j--;
        }
    }
    std::reverse(res.begin(), res.end());
    return res;
}
