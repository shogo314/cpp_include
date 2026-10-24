#pragma once

/**
 * @file segtree-2d.hpp
 * @brief 2次元セグメント木
 */

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/segtree-2d.hpp

/**
 * @brief 2次元セグメント木 (更新する点を先読みする領域木)
 *
 * x 座標のセグメント木の各ノードに、その部分木にある点の y 座標の列と、その上のセグメント木を持たせる。
 *
 * @tparam M 可換モノイド (algebra/monoid.hpp)
 */
template <typename M>
class SegTree2D {
   public:
    using S = typename M::S;

   private:
    M m;
    S op(const S& a, const S& b) const { return m.op(a, b); }
    S e() const { return m.e(); }

    int n;                                  // x 座標の種類数
    std::vector<long long> xs, ys;          // 圧縮した座標
    std::vector<std::vector<int>> node_ys;  // 各ノードにある点の y の番号 (昇順)
    std::vector<std::vector<S>> data;       // 各ノードの y のセグメント木 (大きさは 2 * node_ys[k].size())

    // ノード k の y の番号 yi の位置。なければ -1
    int find_y(int k, int yi) const {
        auto it = std::lower_bound(node_ys[k].begin(), node_ys[k].end(), yi);
        if (it == node_ys[k].end() || *it != yi) return -1;
        return static_cast<int>(it - node_ys[k].begin());
    }
    S get_node(int k, int yi) const {
        int p = find_y(k, yi);
        if (p == -1) return e();
        return data[k][node_ys[k].size() + p];
    }
    int x_index(long long x) const {
        auto it = std::lower_bound(xs.begin(), xs.end(), x);
        assert(it != xs.end() && *it == x);
        return static_cast<int>(it - xs.begin());
    }
    int y_index(long long y) const {
        auto it = std::lower_bound(ys.begin(), ys.end(), y);
        assert(it != ys.end() && *it == y);
        return static_cast<int>(it - ys.begin());
    }
    S prod_node(int k, int ly, int ry) const {
        const auto& v = node_ys[k];
        int len = static_cast<int>(v.size());
        int l = static_cast<int>(std::lower_bound(v.begin(), v.end(), ly) - v.begin()) + len;
        int r = static_cast<int>(std::lower_bound(v.begin(), v.end(), ry) - v.begin()) + len;
        S sml = e(), smr = e();
        while (l < r) {
            if (l & 1) sml = op(sml, data[k][l++]);
            if (r & 1) smr = op(data[k][--r], smr);
            l >>= 1;
            r >>= 1;
        }
        return op(sml, smr);
    }

   public:
    SegTree2D() : SegTree2D(std::vector<std::pair<long long, long long>>()) {}
    /**
     * @brief コンストラクタ
     *
     * @param points set/apply する点 (x, y) の列 (重複してよい)
     * @param m モノイド
     */
    explicit SegTree2D(const std::vector<std::pair<long long, long long>>& points, M m = M()) : m(std::move(m)) {
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
        for (auto [x, y] : points) node_ys[n + x_index(x)].push_back(y_index(y));
        for (int k = n; k < 2 * n; k++) {
            std::sort(node_ys[k].begin(), node_ys[k].end());
            node_ys[k].erase(std::unique(node_ys[k].begin(), node_ys[k].end()), node_ys[k].end());
        }
        for (int k = n - 1; k >= 1; k--) {
            const auto &a = node_ys[2 * k], &b = node_ys[2 * k + 1];
            std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(node_ys[k]));
        }
        data.resize(2 * n);
        for (int k = 1; k < 2 * n; k++) data[k].assign(2 * node_ys[k].size(), e());
    }

    /**
     * @brief 指定された点の値を返す (渡していない点なら単位元)
     */
    S get(long long x, long long y) const {
        auto ix = std::lower_bound(xs.begin(), xs.end(), x);
        auto iy = std::lower_bound(ys.begin(), ys.end(), y);
        if (ix == xs.end() || *ix != x || iy == ys.end() || *iy != y) return e();
        return get_node(n + static_cast<int>(ix - xs.begin()), static_cast<int>(iy - ys.begin()));
    }

    /**
     * @brief 指定された点の値を v に更新する (コンストラクタで渡した点のみ)
     */
    void set(long long x, long long y, const S& v) {
        int yi = y_index(y);
        for (int k = n + x_index(x); k >= 1; k >>= 1) {
            int len = static_cast<int>(node_ys[k].size());
            int p = find_y(k, yi) + len;
            data[k][p] = k >= n ? v : op(get_node(2 * k, yi), get_node(2 * k + 1, yi));
            for (p >>= 1; p >= 1; p >>= 1) data[k][p] = op(data[k][2 * p], data[k][2 * p + 1]);
        }
    }

    /**
     * @brief 指定された点の値に v を作用させる (コンストラクタで渡した点のみ)
     */
    void apply(long long x, long long y, const S& v) {
        int yi = y_index(y);
        // 可換なので、祖先の値にそのまま v を掛ければよい
        for (int k = n + x_index(x); k >= 1; k >>= 1) {
            for (int p = find_y(k, yi) + static_cast<int>(node_ys[k].size()); p >= 1; p >>= 1) data[k][p] = op(data[k][p], v);
        }
    }

    /**
     * @brief [lx, rx) x [ly, ry) にある点の値の総積を返す
     */
    S prod(long long lx, long long rx, long long ly, long long ry) const {
        assert(lx <= rx && ly <= ry);
        int l = static_cast<int>(std::lower_bound(xs.begin(), xs.end(), lx) - xs.begin()) + n;
        int r = static_cast<int>(std::lower_bound(xs.begin(), xs.end(), rx) - xs.begin()) + n;
        int lyi = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), ly) - ys.begin());
        int ryi = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), ry) - ys.begin());
        S res = e();
        while (l < r) {
            if (l & 1) res = op(res, prod_node(l++, lyi, ryi));
            if (r & 1) res = op(res, prod_node(--r, lyi, ryi));
            l >>= 1;
            r >>= 1;
        }
        return res;
    }
};

template <typename M>
SegTree2D(std::vector<std::pair<long long, long long>>, M) -> SegTree2D<M>;

/**
 * @brief RangeMaxQuery
 *
 * @tparam S 型
 */
template <typename S>
using RMaxQ2D = SegTree2D<monoid::Max<S>>;
/**
 * @brief RangeMinQuery
 *
 * @tparam S 型
 */
template <typename S>
using RMinQ2D = SegTree2D<monoid::Min<S>>;
/**
 * @brief RangeSumQuery
 *
 * @tparam S 型
 */
template <typename S>
using RSumQ2D = SegTree2D<monoid::Sum<S>>;
