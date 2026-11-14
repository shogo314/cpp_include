#pragma once
// https://github.com/shogo314/shogo-library/blob/adb06bae4b0e202b3a1fe53ef9f9a1b30077d8cc/src/dp/edit-distance.hpp

/**
 * @file edit-distance.hpp
 * @brief 編集距離 (レーベンシュタイン距離)
 */

#include <algorithm>
#include <cassert>
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

/**
 * @brief 編集距離と、s を t に変える操作列
 */
struct EditDistance {
    enum class Type { Insert,
                      Erase,
                      Replace };
    struct Operation {
        Type type;
        long long source_index;  //!< 元の s の添字。挿入では隙間の位置 [0, |s|]
        long long target_index;  //!< t の添字。削除では隙間の位置 [0, |t|]
    };
    long long distance;
    std::vector<Operation> operations;
};

/**
 * @brief 編集距離と、その最小回数の挿入・削除・置換を求める
 * 時間・メモリ O(nm)。一致する要素を通過する操作は含めない。
 * 操作順に、変更中の列の target_index の位置へ挿入・削除・置換する。
 * 挿入・置換する値は t[target_index]。
 */
template <typename R1, typename R2>
EditDistance edit_distance_restore(const R1& s, const R2& t) {
    auto a = std::begin(s);
    auto b = std::begin(t);
    const long long n = std::end(s) - a, m = std::end(t) - b;
    std::vector<std::vector<long long>> dp(n + 1, std::vector<long long>(m + 1));
    for (long long i = 0; i <= n; ++i) dp[i][0] = i;
    for (long long j = 0; j <= m; ++j) dp[0][j] = j;
    for (long long i = 1; i <= n; ++i) {
        for (long long j = 1; j <= m; ++j) {
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1,
                                 dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1)});
        }
    }
    EditDistance result{dp[n][m], {}};
    result.operations.reserve(result.distance);
    long long i = n, j = m;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && dp[i][j] == dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1)) {
            if (!(a[i - 1] == b[j - 1])) result.operations.push_back({EditDistance::Type::Replace, i - 1, j - 1});
            --i, --j;
        } else if (i > 0 && dp[i][j] == dp[i - 1][j] + 1) {
            result.operations.push_back({EditDistance::Type::Erase, i - 1, j});
            --i;
        } else {
            assert(j > 0);
            result.operations.push_back({EditDistance::Type::Insert, i, j - 1});
            --j;
        }
    }
    std::reverse(result.operations.begin(), result.operations.end());
    return result;
}
