#pragma once

/**
 * @file dual-segtree-2d.hpp
 * @brief 2次元双対セグメント木 (長方形作用、1点取得)
 */

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

#include "../algebra/acted-monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/data_structures/dual-segtree-2d.hpp

/**
 * @brief 2次元双対セグメント木 (取得する点を先読みする)
 *
 * x 座標のセグメント木の各ノードに、その部分木にある点の y 座標の列と、その上の双対セグメント木を持たせる。
 * 作用素を下へ伝えないので、作用素の合成は可換でなければならない
 *
 * @tparam A 作用素付きモノイド (algebra/acted-monoid.hpp)。composition は可換
 */
template <typename A>
class DualSegTree2D {
   public:
    using S = typename A::S;
    using F = typename A::F;

   private:
    A a;
    F composition(const F& f, const F& g) const { return a.composition(f, g); }
    F id() const { return a.id(); }

    int n;                                  // x 座標の種類数
    S init;                                 // 各点の初期値
    std::vector<long long> xs, ys;          // 圧縮した座標
    std::vector<std::vector<int>> node_ys;  // 各ノードにある点の y の番号 (昇順)
    std::vector<std::vector<F>> lazy;       // 各ノードの y の双対セグメント木 (大きさは 2 * node_ys[k].size())

    int lower(int k, int yi) const { return static_cast<int>(std::lower_bound(node_ys[k].begin(), node_ys[k].end(), yi) - node_ys[k].begin()); }

    void construct(const std::vector<std::pair<long long, long long>>& points) {
        for (auto [x, y] : points) {
            xs.push_back(x);
            ys.push_back(y);
        }
        std::sort(xs.begin(), xs.end());
        xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
        std::sort(ys.begin(), ys.end());
        ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
        n = static_cast<int>(xs.size());
        node_ys.resize(2 * n);
        for (auto [x, y] : points) {
            int xi = static_cast<int>(std::lower_bound(xs.begin(), xs.end(), x) - xs.begin());
            node_ys[n + xi].push_back(static_cast<int>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin()));
        }
        for (int k = n; k < 2 * n; k++) {
            std::sort(node_ys[k].begin(), node_ys[k].end());
            node_ys[k].erase(std::unique(node_ys[k].begin(), node_ys[k].end()), node_ys[k].end());
        }
        for (int k = n - 1; k >= 1; k--) {
            const auto &l = node_ys[2 * k], &r = node_ys[2 * k + 1];
            std::set_union(l.begin(), l.end(), r.begin(), r.end(), std::back_inserter(node_ys[k]));
        }
        lazy.resize(2 * n);
        for (int k = 1; k < 2 * n; k++) lazy[k].assign(2 * node_ys[k].size(), id());
    }

   public:
    /**
     * @param points get する点 (x, y) の列 (重複してよい)。各点は S() で初期化する
     * @param a 作用素付きモノイド
     */
    explicit DualSegTree2D(const std::vector<std::pair<long long, long long>>& points = {}, A a = A()) : DualSegTree2D(points, S(), std::move(a)) {}
    /**
     * @param points get する点 (x, y) の列 (重複してよい)
     * @param value 各点の初期値
     * @param a 作用素付きモノイド
     */
    DualSegTree2D(const std::vector<std::pair<long long, long long>>& points, const S& value, A a = A()) : a(std::move(a)), init(value) { construct(points); }

    /**
     * @brief [lx, rx) x [ly, ry) にある点の値に f を作用させる O(log^2 P)
     */
    void apply(long long lx, long long rx, long long ly, long long ry, const F& f) {
        assert(lx <= rx && ly <= ry);
        int l = static_cast<int>(std::lower_bound(xs.begin(), xs.end(), lx) - xs.begin()) + n;
        int r = static_cast<int>(std::lower_bound(xs.begin(), xs.end(), rx) - xs.begin()) + n;
        int lyi = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), ly) - ys.begin());
        int ryi = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), ry) - ys.begin());
        auto apply_node = [&](int k) {
            int len = static_cast<int>(node_ys[k].size());
            for (int i = lower(k, lyi) + len, j = lower(k, ryi) + len; i < j; i >>= 1, j >>= 1) {
                if (i & 1) lazy[k][i] = composition(f, lazy[k][i]), i++;
                if (j & 1) --j, lazy[k][j] = composition(f, lazy[k][j]);
            }
        };
        for (; l < r; l >>= 1, r >>= 1) {
            if (l & 1) apply_node(l++);
            if (r & 1) apply_node(--r);
        }
    }

    /**
     * @brief 指定された点の値を返す (コンストラクタで渡した点のみ) O(log^2 P)
     */
    S get(long long x, long long y) const {
        auto ix = std::lower_bound(xs.begin(), xs.end(), x);
        auto iy = std::lower_bound(ys.begin(), ys.end(), y);
        assert(ix != xs.end() && *ix == x && iy != ys.end() && *iy == y);
        int yi = static_cast<int>(iy - ys.begin());
        F f = id();
        for (int k = n + static_cast<int>(ix - xs.begin()); k >= 1; k >>= 1) {
            int p = lower(k, yi);
            assert(p < static_cast<int>(node_ys[k].size()) && node_ys[k][p] == yi);  // 渡した点か
            for (p += static_cast<int>(node_ys[k].size()); p >= 1; p >>= 1) f = composition(lazy[k][p], f);
        }
        return acted_monoid::apply_mapping(a, f, init, 0, 1);
    }
};

template <typename A>
DualSegTree2D(std::vector<std::pair<long long, long long>>, A) -> DualSegTree2D<A>;
template <typename A>
DualSegTree2D(std::vector<std::pair<long long, long long>>, typename A::S, A) -> DualSegTree2D<A>;

// 別名 (作用素の合成が可換なものだけ。DualSegTree の別名と同じく Min 系を実体にする)

/**
 * @brief 長方形加算、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RAddQ2D = DualSegTree2D<acted_monoid::AddMin<S>>;
/**
 * @brief 長方形 chmin、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RChminQ2D = DualSegTree2D<acted_monoid::ChminMin<S>>;
/**
 * @brief 長方形 chmax、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RChmaxQ2D = DualSegTree2D<acted_monoid::ChmaxMax<S>>;
