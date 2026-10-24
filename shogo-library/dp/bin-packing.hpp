#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/dp/bin-packing.hpp

/**
 * @file bin-packing.hpp
 * @brief 瓶詰め問題 (ビンパッキング) の最小の瓶の数 (品物が 20 個程度まで)
 */

#include <utility>
#include <vector>

/**
 * @brief 容量 C の瓶に重さ w_i の品物をすべて詰めるのに必要な瓶の最小個数 (入らない品物があれば -1) O(2^n n)
 * 品物を 1 つずつ入れる順番をすべて試し、「最後の瓶に入るなら入れ、入らなければ新しい瓶を開ける」とする
 * 最適な詰め方も瓶ごとに順に入れれば再現できるので、品物の集合ごとに (瓶の数, 最後の瓶の重さ) の辞書順最小を持てばよい
 */
inline long long bin_packing(const std::vector<long long>& w, long long C) {
    const int n = w.size();
    for (long long x : w)
        if (x > C) return -1;
    // dp[S] = 集合 S を詰めたときの (瓶の数, 最後の瓶に入っている重さ) の辞書順最小。最初は最後の瓶が満杯とみなす
    std::vector<std::pair<int, long long>> dp(1 << n, {n + 1, 0});
    dp[0] = {0, C};
    for (int S = 0; S < (1 << n); S++) {
        auto [bins, fill] = dp[S];
        if (bins > n) continue;
        for (int i = 0; i < n; i++) {
            if (S >> i & 1) continue;
            std::pair<int, long long> next = fill + w[i] <= C ? std::make_pair(bins, fill + w[i]) : std::make_pair(bins + 1, w[i]);
            if (next < dp[S | 1 << i]) dp[S | 1 << i] = next;
        }
    }
    return dp[(1 << n) - 1].first;
}
