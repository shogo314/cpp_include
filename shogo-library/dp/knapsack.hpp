#pragma once
// https://github.com/shogo314/shogo-library/blob/main/src/dp/knapsack.hpp

/**
 * @file knapsack.hpp
 * @brief ナップサック問題
 */

#include <algorithm>
#include <cassert>
#include <deque>
#include <limits>
#include <utility>
#include <vector>

/*
 * 品物 i は価値 v[i] と重さ w[i] を持ち (どちらも 0 以上)、重さの和が W 以下になるように選んで、価値の和の最大値を求める
 * 答え (価値の和の最大値) は long long に収まること
 */

namespace internal_knapsack {
inline void check(const std::vector<long long>& v, const std::vector<long long>& w, long long W) {
    assert(v.size() == w.size() && W >= 0);
    for (int i = 0; i < (int)v.size(); i++) assert(v[i] >= 0 && w[i] >= 0);
}

// 各アルゴリズムの計算量の見積もり (大きすぎるものは inf)
inline long double cost_by_weight(int n, long long W) { return (long double)(n + 1) * ((long double)W + 1); }
inline long double cost_by_value(int n, long double value_sum) { return (long double)(n + 1) * (value_sum + 1); }
inline long double cost_meet_in_the_middle(int n) {
    if (n > 80) return std::numeric_limits<long double>::infinity();
    return (long double)(n + 1) * (long double)(1ULL << ((n + 1) / 2));
}

// 個数制限付きの品物を、個数 1, 2, 4, ..., 残り の組に分けて 0/1 の品物にする
inline void binary_split(const std::vector<long long>& v, const std::vector<long long>& w, const std::vector<long long>& m,
                         long long W, std::vector<long long>& v2, std::vector<long long>& w2) {
    for (int i = 0; i < (int)v.size(); i++) {
        long long c = m[i];
        // W を超える個数は使えない
        if (w[i] > 0) c = std::min(c, W / w[i]);
        for (long long k = 1; c > 0; k *= 2) {
            long long t = std::min(k, c);
            v2.push_back(v[i] * t);
            w2.push_back(w[i] * t);
            c -= t;
        }
    }
}
}  // namespace internal_knapsack

/**
 * @brief 0/1 ナップサック (重さについての DP) O(N W)
 */
inline long long knapsack_by_weight(const std::vector<long long>& v, const std::vector<long long>& w, long long W) {
    internal_knapsack::check(v, w, W);
    // dp[j] は重さの和が j 以下のときの価値の和の最大値
    std::vector<long long> dp(W + 1, 0);
    for (int i = 0; i < (int)v.size(); i++) {
        if (w[i] > W) continue;
        for (long long j = W; j >= w[i]; j--) dp[j] = std::max(dp[j], dp[j - w[i]] + v[i]);
    }
    return dp[W];
}

/**
 * @brief 0/1 ナップサック (価値についての DP) O(N V) (V は価値の和)
 */
inline long long knapsack_by_value(const std::vector<long long>& v, const std::vector<long long>& w, long long W) {
    internal_knapsack::check(v, w, W);
    long long value_sum = 0;
    for (long long x : v) value_sum += x;
    // dp[s] は価値の和がちょうど s のときの重さの和の最小値 (W を超えるものは INF)
    const long long INF = std::numeric_limits<long long>::max();
    std::vector<long long> dp(value_sum + 1, INF);
    dp[0] = 0;
    for (int i = 0; i < (int)v.size(); i++) {
        if (w[i] > W) continue;
        for (long long s = value_sum; s >= v[i]; s--) {
            long long prev = dp[s - v[i]];
            if (prev != INF && prev <= W - w[i]) dp[s] = std::min(dp[s], prev + w[i]);
        }
    }
    for (long long s = value_sum;; s--) {
        if (dp[s] <= W) return s;
    }
}

/**
 * @brief 0/1 ナップサック (半分全列挙) O(2^(N/2) N)
 * N が 40 程度までなら、重さや価値が大きくてもよい
 */
inline long long knapsack_meet_in_the_middle(const std::vector<long long>& v, const std::vector<long long>& w, long long W) {
    internal_knapsack::check(v, w, W);
    int n = v.size();
    // 品物 [l, r) の部分集合のうち重さの和が W 以下のものを、(重さ, 価値) の重さの昇順で列挙する
    auto enumerate = [&](int l, int r) {
        std::vector<std::pair<long long, long long>> res = {{0, 0}}, add, merged;
        for (int i = l; i < r; i++) {
            add.clear();
            for (auto [x, y] : res) {
                if (w[i] <= W - x) add.emplace_back(x + w[i], y + v[i]);
            }
            merged.resize(res.size() + add.size());
            std::merge(res.begin(), res.end(), add.begin(), add.end(), merged.begin());
            std::swap(res, merged);
        }
        return res;
    };
    auto a = enumerate(0, n / 2), b = enumerate(n / 2, n);
    // b の価値を、重さがそれ以下のものの最大値に置き換える
    for (int i = 1; i < (int)b.size(); i++) b[i].second = std::max(b[i].second, b[i - 1].second);
    long long res = 0;
    int j = b.size() - 1;
    for (auto [x, y] : a) {
        while (j >= 0 && b[j].first > W - x) j--;
        if (j < 0) break;
        res = std::max(res, y + b[j].second);
    }
    return res;
}

/**
 * @brief 0/1 ナップサック。入力の大きさから、計算量が最も小さいアルゴリズムを選ぶ
 *
 * knapsack_by_weight O(N W)、knapsack_by_value O(N V)、knapsack_meet_in_the_middle O(2^(N/2) N) のいずれか
 */
inline long long knapsack(const std::vector<long long>& v, const std::vector<long long>& w, long long W) {
    using namespace internal_knapsack;
    int n = v.size();
    long double value_sum = 0;
    for (long long x : v) value_sum += x;
    long double cw = cost_by_weight(n, W), cv = cost_by_value(n, value_sum), cm = cost_meet_in_the_middle(n);
    if (cw <= cv && cw <= cm) return knapsack_by_weight(v, w, W);
    if (cv <= cm) return knapsack_by_value(v, w, W);
    return knapsack_meet_in_the_middle(v, w, W);
}

/**
 * @brief 個数制限なしナップサック (同じ品物を何個でも選べる) O(N W)
 */
inline long long unbounded_knapsack(const std::vector<long long>& v, const std::vector<long long>& w, long long W) {
    internal_knapsack::check(v, w, W);
    for (int i = 0; i < (int)v.size(); i++) assert((w[i] > 0 || v[i] == 0) && "unbounded_knapsack: infinite value");
    std::vector<long long> dp(W + 1, 0);
    for (int i = 0; i < (int)v.size(); i++) {
        if (w[i] == 0 || w[i] > W) continue;
        for (long long j = w[i]; j <= W; j++) dp[j] = std::max(dp[j], dp[j - w[i]] + v[i]);
    }
    return dp[W];
}

/**
 * @brief 個数制限付きナップサック (品物 i は m[i] 個まで選べる、重さについての DP) O(N W)
 * 重さを w[i] で割った余りごとに、スライド最大値を使う
 */
inline long long bounded_knapsack_by_weight(const std::vector<long long>& v, const std::vector<long long>& w,
                                            const std::vector<long long>& m, long long W) {
    internal_knapsack::check(v, w, W);
    assert(m.size() == v.size());
    std::vector<long long> dp(W + 1, 0), cur;
    std::deque<std::pair<long long, long long>> dq;  // (t, dp[r + t w] - t v) で、値が減少する順
    for (int i = 0; i < (int)v.size(); i++) {
        assert(m[i] >= 0);
        if (w[i] == 0) {
            for (auto& x : dp) x += v[i] * m[i];
            continue;
        }
        if (w[i] > W) continue;
        for (long long r = 0; r < w[i]; r++) {
            // j = r + k w について dp[j] = max_{k - m <= t <= k} (dp[r + t w] - t v) + k v
            cur.clear();
            for (long long j = r; j <= W; j += w[i]) cur.push_back(dp[j]);
            dq.clear();
            for (long long k = 0; k < (long long)cur.size(); k++) {
                long long val = cur[k] - k * v[i];
                while (!dq.empty() && dq.back().second <= val) dq.pop_back();
                dq.emplace_back(k, val);
                if (dq.front().first < k - m[i]) dq.pop_front();
                dp[r + k * w[i]] = dq.front().second + k * v[i];
            }
        }
    }
    return dp[W];
}

/**
 * @brief 個数制限付きナップサック (二進分割で 0/1 ナップサックにして、価値についての DP) O(V sum log m[i]) (V は v[i] m[i] の和)
 */
inline long long bounded_knapsack_by_value(const std::vector<long long>& v, const std::vector<long long>& w,
                                           const std::vector<long long>& m, long long W) {
    internal_knapsack::check(v, w, W);
    assert(m.size() == v.size());
    std::vector<long long> v2, w2;
    internal_knapsack::binary_split(v, w, m, W, v2, w2);
    return knapsack_by_value(v2, w2, W);
}

/**
 * @brief 個数制限付きナップサック。入力の大きさから、計算量が最も小さいアルゴリズムを選ぶ
 *
 * bounded_knapsack_by_weight O(N W)、二進分割した後の 0/1 ナップサックの価値についての DP か半分全列挙のいずれか
 */
inline long long bounded_knapsack(const std::vector<long long>& v, const std::vector<long long>& w,
                                  const std::vector<long long>& m, long long W) {
    using namespace internal_knapsack;
    check(v, w, W);
    assert(m.size() == v.size());
    int n = v.size();
    // 二進分割した後の品物の個数と価値の和
    long double pieces = 0, value_sum = 0;
    for (int i = 0; i < n; i++) {
        long long c = m[i];
        if (w[i] > 0) c = std::min(c, W / w[i]);
        for (long long k = 1, rest = c; rest > 0; k *= 2) {
            pieces++;
            rest -= std::min(k, rest);
        }
        value_sum += (long double)v[i] * c;
    }
    long double cw = cost_by_weight(n, W);
    long double cv = pieces > 1e9 ? std::numeric_limits<long double>::infinity() : cost_by_value((int)pieces, value_sum);
    long double cm = pieces > 80 ? std::numeric_limits<long double>::infinity() : cost_meet_in_the_middle((int)pieces);
    if (cw <= cv && cw <= cm) return bounded_knapsack_by_weight(v, w, m, W);
    std::vector<long long> v2, w2;
    binary_split(v, w, m, W, v2, w2);
    if (cv <= cm) return knapsack_by_value(v2, w2, W);
    return knapsack_meet_in_the_middle(v2, w2, W);
}
