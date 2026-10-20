#pragma once

/**
 * @file segtree-2d.hpp
 * @brief 2次元セグメント木
 */

#include <algorithm>
#include <cassert>
#include <tuple>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/data_structures/segtree-2d.hpp

/**
 * @brief 2次元セグメント木 (更新・取得する座標を先読みして圧縮する)
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

    long long h, w, sz_x, sz_y;
    int height_x, height_y;
    std::vector<std::vector<int>> y_indices;  // 各 x のノードが持つ y のノード番号 (メモリを抑えるため int で持つ)
    std::vector<std::vector<S>> data_compressed;

    S get_y(long long kx, long long ky) {
        ky += sz_y;
        auto it = std::lower_bound(y_indices[kx].begin(), y_indices[kx].end(), ky);
        if (it == y_indices[kx].end() || *it != ky) return e();
        return data_compressed[kx][it - y_indices[kx].begin()];
    }

    S updated_y(long long kx, long long ky, int child_y_idx, bool child_y_right, const S& child_val) {
        if (!child_y_right && child_y_idx + 1 < static_cast<int>(y_indices[kx].size()) && y_indices[kx][child_y_idx + 1] == ky * 2 + 1) {
            return op(child_val, data_compressed[kx][child_y_idx + 1]);
        }
        if (child_y_right && child_y_idx - 1 >= 0 && y_indices[kx][child_y_idx - 1] == ky * 2) {
            return op(data_compressed[kx][child_y_idx - 1], child_val);
        }
        return child_val;
    }

    void update_tree_y(long long kx, long long ky, const S& val) {
        ky += sz_y;
        int y_idx = std::lower_bound(y_indices[kx].begin(), y_indices[kx].end(), ky) - y_indices[kx].begin();
        S tmp_val = val;
        data_compressed[kx][y_idx] = tmp_val;
        for (int i = 1; i <= height_y; i++) {
            int child_y_idx = y_idx;
            y_idx = std::lower_bound(y_indices[kx].begin(), y_indices[kx].begin() + child_y_idx, ky >> i) - y_indices[kx].begin();
            tmp_val = data_compressed[kx][y_idx] = updated_y(kx, ky >> i, child_y_idx, ky >> (i - 1) & 1, tmp_val);
        }
    }

    S updated_x(long long kx, long long ky, bool child_x_right, const S& child_val) {
        if (!child_x_right) {
            return op(child_val, get_y(kx * 2 + 1, ky));
        } else {
            return op(get_y(kx * 2, ky), child_val);
        }
    }

    void update_tree_x(long long kx, long long ky, const S& val) {
        kx += sz_x;
        S tmp_val = val;
        update_tree_y(kx, ky, tmp_val);
        for (int i = 1; i <= height_x; i++) {
            tmp_val = updated_x(kx >> i, ky, kx >> (i - 1) & 1, tmp_val);
            update_tree_y(kx >> i, ky, tmp_val);
        }
    }

    S prod_y(long long kx, long long ly, long long ry) {
        S left_prod = e(), right_prod = e();
        ly += sz_y;
        ry += sz_y;
        auto itr = y_indices[kx].end();
        while (ly < ry) {
            if (ry & 1) {
                --ry;
                itr = std::lower_bound(y_indices[kx].begin(), itr, ry);
                right_prod = op(data_compressed[kx][itr - y_indices[kx].begin()], right_prod);
            }
            if (ly & 1) {
                itr = std::lower_bound(y_indices[kx].begin(), itr, ly);
                left_prod = op(left_prod, data_compressed[kx][itr - y_indices[kx].begin()]);
                ++ly;
            }
            ly >>= 1;
            ry >>= 1;
        }
        return op(left_prod, right_prod);
    }

    S prod_x(long long lx, long long rx, long long ly, long long ry) {
        S left_prod = e(), right_prod = e();
        lx += sz_x;
        rx += sz_x;
        while (lx < rx) {
            if (lx & 1) {
                left_prod = op(left_prod, prod_y(lx++, ly, ry));
            }
            if (rx & 1) {
                right_prod = op(prod_y(--rx, ly, ry), right_prod);
            }
            lx >>= 1;
            rx >>= 1;
        }
        return op(left_prod, right_prod);
    }

    void construct_data(const std::vector<std::tuple<long long, long long>>& set_query_indices, const std::vector<std::tuple<long long, long long, long long, long long>>& prod_query_indices) {
        sz_x = 1;
        height_x = 0;
        while (sz_x < h) {
            sz_x <<= 1;
            ++height_x;
        }
        sz_y = 1;
        height_y = 0;
        while (sz_y < w) {
            sz_y <<= 1;
            ++height_y;
        }
        data_compressed.resize(sz_x * 2);
        y_indices.resize(sz_x * 2);

        for (auto [x, y] : set_query_indices) {
            x += sz_x;
            std::vector<long long> x_indices_tmp;
            for (int i = 0; i <= height_x; ++i) {
                x_indices_tmp.push_back(x);
                x >>= 1;
            }
            y += sz_y;
            std::vector<int> y_indices_tmp;
            for (int i = 0; i <= height_y; ++i) {
                y_indices_tmp.push_back(static_cast<int>(y));
                y >>= 1;
            }
            for (long long x : x_indices_tmp) {
                for (int y : y_indices_tmp) {
                    y_indices[x].push_back(y);
                }
            }
        }
        for (auto [lx, rx, ly, ry] : prod_query_indices) {
            std::vector<long long> x_indices_tmp;
            lx += sz_x;
            rx += sz_x;
            while (lx < rx) {
                if (lx & 1) x_indices_tmp.push_back(lx++);
                if (rx & 1) x_indices_tmp.push_back(--rx);
                lx >>= 1;
                rx >>= 1;
            }
            ly += sz_y;
            ry += sz_y;
            std::vector<int> y_indices_tmp;
            while (ly < ry) {
                if (ly & 1) y_indices_tmp.push_back(static_cast<int>(ly++));
                if (ry & 1) y_indices_tmp.push_back(static_cast<int>(--ry));
                ly >>= 1;
                ry >>= 1;
            }
            for (long long x : x_indices_tmp) {
                for (int y : y_indices_tmp) {
                    y_indices[x].push_back(y);
                }
            }
        }

        for (long long x = 1; x < sz_x * 2; x++) {
            std::sort(y_indices[x].begin(), y_indices[x].end());
            y_indices[x].erase(std::unique(y_indices[x].begin(), y_indices[x].end()), y_indices[x].end());
        }
        for (long long i = 1; i < sz_x * 2; ++i) {
            data_compressed[i].assign(y_indices[i].size(), e());
        }
    }

   public:
    SegTree2D() = default;
    /**
     * @brief コンストラクタ
     *
     * @param h x座標の範囲 [0, h)
     * @param w y座標の範囲 [0, w)
     * @param set_query_indices set/apply する座標 (x, y) の列
     * @param prod_query_indices prod する範囲 (lx, rx, ly, ry) の列
     * @param m モノイド
     */
    explicit SegTree2D(long long h, long long w, const std::vector<std::tuple<long long, long long>>& set_query_indices, const std::vector<std::tuple<long long, long long, long long, long long>>& prod_query_indices, M m = M())
        : m(std::move(m)), h(h), w(w) {
        construct_data(set_query_indices, prod_query_indices);
    }

    /**
     * @brief 指定された要素の値を返す
     * @param kx x座標
     * @param ky y座標
     * @return 要素の値
     */
    S get(long long kx, long long ky) {
        assert(0 <= kx && kx < h && 0 <= ky && ky < w);
        kx += sz_x;
        return get_y(kx, ky);
    }

    /**
     * @brief 指定された要素の値をxに更新する
     * @param kx x座標
     * @param ky y座標
     * @param x 代入する値
     */
    void set(long long kx, long long ky, const S& x) {
        assert(0 <= kx && kx < h && 0 <= ky && ky < w);
        update_tree_x(kx, ky, x);
    }

    /**
     * @brief 指定された要素の値にxを作用させる
     *
     * @param kx x座標
     * @param ky y座標
     * @param x 作用素
     */
    void apply(long long kx, long long ky, const S& x) {
        set(kx, ky, op(get(kx, ky), x));
    }

    /**
     * @brief [lx, rx)x[ly, ry)の範囲の要素の総積を返す
     *
     * @param lx x座標の半開区間の開始
     * @param rx x座標の半開区間の終端
     * @param ly y座標の半開区間の開始
     * @param ry y座標の半開区間の終端
     */
    S prod(long long lx, long long rx, long long ly, long long ry) {
        assert(0 <= lx && lx <= rx && rx <= h);
        assert(0 <= ly && ly <= ry && ry <= w);
        return prod_x(lx, rx, ly, ry);
    }
};

template <typename M>
SegTree2D(long long, long long, std::vector<std::tuple<long long, long long>>, std::vector<std::tuple<long long, long long, long long, long long>>, M) -> SegTree2D<M>;

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
