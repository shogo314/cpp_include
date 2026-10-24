#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/geometry/rectangle-union.hpp

/**
 * @file rectangle-union.hpp
 * @brief 長方形の和集合の面積
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <tuple>
#include <vector>

/**
 * @brief 軸に平行な長方形 [l, r) × [d, u) の和集合の面積 (走査線) O(n log n)
 *
 * x の小さい順に走査し、y 方向の各区間を何枚の長方形が覆っているかをセグメント木で持つ。
 * 覆う枚数が正の区間だけを長さに数えるので、遅延伝播はいらない
 * @param rects 長方形 {l, d, r, u} のリスト (l <= r、d <= u)
 * @return T 面積
 */
template <typename T>
T rectangle_union_area(const std::vector<std::array<T, 4>>& rects) {
    std::vector<T> ys;
    for (const auto& [l, d, r, u] : rects) {
        assert(l <= r && d <= u);
        ys.push_back(d), ys.push_back(u);
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    const int m = static_cast<int>(ys.size()) - 1;  // y 方向の区間 [ys[i], ys[i+1]) の個数
    if (m <= 0) return T(0);
    // (x, 加える枚数, y の区間 [lo, hi))
    std::vector<std::tuple<T, int, int, int>> events;
    for (const auto& [l, d, r, u] : rects) {
        if (l == r || d == u) continue;
        int lo = std::lower_bound(ys.begin(), ys.end(), d) - ys.begin(), hi = std::lower_bound(ys.begin(), ys.end(), u) - ys.begin();
        events.emplace_back(l, 1, lo, hi);
        events.emplace_back(r, -1, lo, hi);
    }
    std::sort(events.begin(), events.end());
    int sz = 1;
    while (sz < m) sz <<= 1;
    std::vector<int> cover(2 * sz, 0);  // そのノードの区間全体を覆う長方形の枚数
    std::vector<T> len(2 * sz, T(0));   // そのノードの区間で覆われている長さ
    // ノード k (区間 [a, b)) に [lo, hi) を v 枚足す
    auto add = [&](auto&& self, int k, int a, int b, int lo, int hi, int v) -> void {
        if (hi <= a || b <= lo) return;
        if (lo <= a && b <= hi) {
            cover[k] += v;
        } else {
            int c = (a + b) / 2;
            self(self, 2 * k, a, c, lo, hi, v);
            self(self, 2 * k + 1, c, b, lo, hi, v);
        }
        if (cover[k] > 0)
            len[k] = ys[std::min(b, m)] - ys[std::min(a, m)];
        else
            len[k] = k >= sz ? T(0) : len[2 * k] + len[2 * k + 1];
    };
    T res = 0;
    for (int i = 0; i < static_cast<int>(events.size()); i++) {
        auto [x, v, lo, hi] = events[i];
        add(add, 1, 0, sz, lo, hi, v);
        if (i + 1 < static_cast<int>(events.size())) res += len[1] * (std::get<0>(events[i + 1]) - x);
    }
    return res;
}
