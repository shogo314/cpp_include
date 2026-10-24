#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/range-mode.hpp

/**
 * @file range-mode.hpp
 * @brief 区間の最頻値
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>
#include <vector>

/**
 * @brief 固定した列について、区間の最頻値とその出現回数をオンラインで答える
 *
 * 長さ B のブロックに分け、ブロックの区間ごとの最頻値を前計算する。クエリでは、間のブロックの最頻値から始めて、
 * 端の要素ごとに「その値の出現位置の列」で出現回数が 1 つ増やせるかを調べる (増やす回数の合計は端の長さ以下)
 * 構築 O(n sqrt(n))、クエリ O(sqrt(n))、メモリ O(n)
 * 例:
 * RangeMode rm(a);
 * auto [value, count] = rm.mode(l, r);
 *
 * @tparam T 値の型 (operator< で比べられること)
 */
template <typename T>
class RangeMode {
    long long n, B;
    std::vector<T> vals;                      // 値の種類 (昇順)
    std::vector<long long> id;                // id[i]: a[i] の vals での番号
    std::vector<long long> rank;              // rank[i]: a[i] と同じ値のうち i より前にあるものの個数
    std::vector<std::vector<long long>> pos;  // pos[v]: 値 v の出現位置 (昇順)
    // best[i][j]: ブロック i から j まで (両端を含む) の最頻値の番号と出現回数
    std::vector<std::vector<std::pair<long long, long long>>> best;

   public:
    /**
     * @brief 列 a から作る
     */
    explicit RangeMode(const std::vector<T>& a) : n(a.size()), vals(a), id(n), rank(n) {
        std::sort(vals.begin(), vals.end());
        vals.erase(std::unique(vals.begin(), vals.end()), vals.end());
        pos.resize(vals.size());
        for (long long i = 0; i < n; i++) {
            id[i] = std::lower_bound(vals.begin(), vals.end(), a[i]) - vals.begin();
            rank[i] = pos[id[i]].size();
            pos[id[i]].push_back(i);
        }
        B = std::max(1LL, static_cast<long long>(std::sqrt(static_cast<double>(n))));
        const long long k = (n + B - 1) / B;
        best.assign(k, std::vector<std::pair<long long, long long>>(k));
        std::vector<long long> cnt(vals.size());
        for (long long i = 0; i < k; i++) {
            std::fill(cnt.begin(), cnt.end(), 0);
            std::pair<long long, long long> cur = {-1, 0};
            for (long long j = i * B; j < n; j++) {
                if (++cnt[id[j]] > cur.second) cur = {id[j], cnt[id[j]]};
                if (j % B == B - 1 || j == n - 1) best[i][j / B] = cur;
            }
        }
    }
    /**
     * @brief 要素数
     */
    long long size() const { return n; }
    /**
     * @brief a[l], ..., a[r-1] の最頻値の 1 つと、その出現回数 {値, 回数}
     * @pre l < r
     */
    std::pair<T, long long> mode(long long l, long long r) const {
        assert(0 <= l && l < r && r <= n);
        // 間にあるブロック [bl, br]。なければ空にして、すべて端として扱う
        const long long bl = (l + B - 1) / B, br = r / B - 1;
        long long v = -1, f = 0, mid_l = r, mid_r = r;
        if (bl <= br) v = best[bl][br].first, f = best[bl][br].second, mid_l = bl * B, mid_r = (br + 1) * B;
        // 左端: i から右に f + 1 個目の出現が r より前にあれば増やす
        for (long long i = l; i < mid_l; i++) {
            const auto& p = pos[id[i]];
            while (rank[i] + f < static_cast<long long>(p.size()) && p[rank[i] + f] < r) v = id[i], f++;
        }
        // 右端: i から左に f + 1 個目の出現が l 以上なら増やす
        for (long long i = mid_r; i < r; i++) {
            const auto& p = pos[id[i]];
            while (rank[i] - f >= 0 && p[rank[i] - f] >= l) v = id[i], f++;
        }
        return {vals[v], f};
    }
};

template <typename T>
RangeMode(std::vector<T>) -> RangeMode<T>;
