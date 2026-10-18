#pragma once

/**
 * @file subset-sum-count.hpp
 * @brief 部分和の個数 (和が s になる部分集合の個数)
 */

#include <cassert>
#include <climits>
#include <cmath>
#include <vector>

#include "../math/fps.hpp"
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/dp/subset-sum-count.hpp

/**
 * @brief 和が s になる部分集合 (各要素は 1 回まで) の個数を、s = 0, 1, ..., S について求める (DP) O(N S)
 * @tparam T 個数の型 (modint や long long)
 * @param a 0 以上の整数の列
 */
template <typename T>
std::vector<T> subset_sum_count_dp(const std::vector<long long>& a, long long S) {
    assert(0 <= S && S < INT_MAX);
    const int t = static_cast<int>(S);
    std::vector<T> dp(t + 1, T(0));
    dp[0] = T(1);
    for (long long x : a) {
        assert(x >= 0);
        if (x > t) continue;
        if (x == 0) {
            // 選ぶか選ばないかで 2 倍
            for (auto& y : dp) y += y;
            continue;
        }
        for (int s = t; s >= x; s--) dp[s] += dp[s - x];
    }
    return dp;
}

/**
 * @brief 和が s になる部分集合 (各要素は 1 回まで) の個数を、s = 0, 1, ..., S について求める (形式的冪級数の exp) O(S log S)
 *
 * prod (1 + x^a_i) = exp(sum_i log(1 + x^a_i)) を使う
 * @tparam mint modint。mod は S より大きい素数であること
 * @param a 0 以上の整数の列
 */
template <typename mint>
std::vector<mint> subset_sum_count_fps(const std::vector<long long>& a, long long S) {
    assert(0 <= S && S < INT_MAX);
    const int t = static_cast<int>(S);
    std::vector<int> cnt(t + 1);
    mint zero_factor = 1;  // 0 の要素は、選ぶか選ばないかで 2 倍
    for (long long x : a) {
        assert(x >= 0);
        if (x == 0)
            zero_factor += zero_factor;
        else if (x <= t)
            cnt[x]++;
    }
    std::vector<mint> inv(t + 1, mint(1));
    for (int i = 2; i <= t; i++) inv[i] = -inv[mint::mod() % i] * mint(mint::mod() / i);
    // log(1 + x^k) = sum_{j >= 1} (-1)^(j + 1) x^(k j) / j
    FPS<mint> f(t + 1);
    for (int k = 1; k <= t; k++) {
        if (cnt[k] == 0) continue;
        for (int j = 1; k * j <= t; j++) {
            if (j & 1)
                f[k * j] += mint(cnt[k]) * inv[j];
            else
                f[k * j] -= mint(cnt[k]) * inv[j];
        }
    }
    FPS<mint> g = exp(f, t + 1);
    std::vector<mint> res(t + 1);
    for (int s = 0; s <= t && s < (int)g.size(); s++) res[s] = g[s] * zero_factor;
    return res;
}

/**
 * @brief 和が s になる部分集合 (各要素は 1 回まで) の個数を、s = 0, 1, ..., S について求める
 *
 * 計算量が小さい方を選ぶ: subset_sum_count_dp O(N S) か subset_sum_count_fps O(S log S)
 * @tparam mint modint。mod は S より大きい素数であること
 */
template <typename mint>
std::vector<mint> subset_sum_count(const std::vector<long long>& a, long long S) {
    // 実測では、DP は N S あたり約 0.2 ns、exp は S log S あたり約 100 ns なので、exp を 500 倍と見積もる
    double dp_cost = (double)a.size() * (S + 1);
    double fps_cost = 500.0 * (S + 1) * std::log2(S + 2);
    if (dp_cost <= fps_cost) return subset_sum_count_dp<mint>(a, S);
    return subset_sum_count_fps<mint>(a, S);
}
