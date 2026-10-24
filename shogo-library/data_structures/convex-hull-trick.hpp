#pragma once

/**
 * @file convex-hull-trick.hpp
 * @brief Convex Hull Trick
 *
 * 一次関数で表される直線または線分の集合を管理し、あるxに対する最小値を求める
 */

#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/data_structures/convex-hull-trick.hpp

/**
 * @brief 追加される傾きが単調とは限らない場合のConvex Hull Trick
 *
 * @tparam T 値の型
 */
template <typename T, ENABLE_IF_T(std::is_scalar_v<T>)>
class LiChaoTree {
    long long n, sz;
    int height;
    std::vector<T> xs, as, bs;
    std::vector<bool> exist;  // ノードが直線を持つか

    void update(T a, T b, long long k, int h) {
        long long l = (k << h) & (sz - 1);
        long long r = l + (1LL << h);

        while (1) {
            if (!exist[k]) {
                as[k] = a, bs[k] = b, exist[k] = true;
                return;
            }
            long long m = (l + r) >> 1;
            T xl = xs[l], xm = xs[m];
            bool l_update = a * xl + b < as[k] * xl + bs[k];
            bool m_update = a * xm + b < as[k] * xm + bs[k];

            if (m_update) {
                std::swap(as[k], a);
                std::swap(bs[k], b);
            }
            if (h == 0) break;
            if (l_update != m_update) {
                k = k * 2;
                r = m;
            } else {
                k = k * 2 + 1;
                l = m;
            }
            h--;
        }
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * @param xs 最小値クエリのx座標や線分追加クエリの両端のx座標を先読みしてソートした配列
     */
    explicit LiChaoTree(const std::vector<T>& xs) : n(xs.size()), xs(xs) {
        assert(!xs.empty());
        sz = 1, height = 0;
        while (sz < static_cast<long long>(xs.size())) {
            sz <<= 1;
            height++;
        }
        this->xs.resize(sz, xs.back());
        as.assign(sz * 2, 0);
        bs.assign(sz * 2, 0);
        exist.assign(sz * 2, false);
    }

    /**
     * @brief 直線y=ax+bの追加
     *
     * @param a 傾き
     * @param b 切片
     */
    void add_line(T a, T b) {
        update(a, b, 1, height);
    }
    /**
     * @brief 線分y=ax+b , x∈[xs[l],xs[r])の追加
     *
     * @param a 傾き
     * @param b 切片
     * @param l 左端(xsのインデックス)
     * @param r xの上限(xsのインデックス)
     */
    void add_segment(T a, T b, long long l, long long r) {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return;
        l += sz, r += sz;
        int h = 0;
        while (l < r) {
            if (l & 1) update(a, b, l++, h);
            if (r & 1) update(a, b, --r, h);
            l >>= 1, r >>= 1, h++;
        }
    }

    /**
     * @brief x=xs[i]における最小値を求める
     *
     * @param i x座標(xsのインデックス)
     * @return std::optional<T> 最小値。直線がなければ std::nullopt
     */
    std::optional<T> get_min(long long i) const {
        assert(0 <= i && i < n);
        T x = xs[i];
        std::optional<T> res;
        for (long long k = i + sz; k >= 1; k >>= 1) {
            if (!exist[k]) continue;
            T tmp = as[k] * x + bs[k];
            if (!res || tmp < *res) res = tmp;
        }
        return res;
    }
};

/**
 * @brief x 座標を先読みしない Li Chao Tree (x は [lo, hi) の整数)
 *
 * 必要になった頂点だけを作るので、x の範囲が 10^9 程度でも使える
 * 頂点は、直線 1 本につき 1 個以下、線分 1 本につき 2 log (hi - lo) 個程度増える。線分を何十万本も足すなら、x 座標を先読みする LiChaoTree の方が速い
 * 例:
 * DynamicLiChaoTree<long long> cht(-1000000000, 1000000001);
 * cht.add_line(2, 3); cht.add_segment(-1, 0, 0, 10);  // y = -x (x ∈ [0, 10))
 * cht.get_min(5);  // -5
 *
 * @tparam T 値の型 (a x + b があふれないこと)
 */
template <typename T>
class DynamicLiChaoTree {
    struct Node {
        T a, b;
        bool exist = false;
        int left = -1, right = -1;
    };
    long long lo, hi;
    std::vector<Node> nodes;

    static T eval(T a, T b, long long x) { return a * static_cast<T>(x) + b; }
    int child(int k, bool right) {
        int& c = right ? nodes[k].right : nodes[k].left;
        if (c == -1) {
            // push_back で nodes が伸びると参照 c が無効になるので、先に番号を決めておく
            int id = nodes.size();
            c = id;
            nodes.emplace_back();
            return id;
        }
        return c;
    }
    // 頂点 k (区間 [l, r)) に直線を入れる。今の直線と比べて、区間の中点で良い方を残し、悪い方を片側の子へ送る
    void update(int k, long long l, long long r, T a, T b) {
        while (true) {
            Node& nd = nodes[k];
            if (!nd.exist) {
                nd.a = a, nd.b = b, nd.exist = true;
                return;
            }
            const long long m = l + (r - l) / 2;
            bool l_better = eval(a, b, l) < eval(nd.a, nd.b, l);
            bool m_better = eval(a, b, m) < eval(nd.a, nd.b, m);
            if (m_better) std::swap(nd.a, a), std::swap(nd.b, b);
            if (r - l == 1) return;
            // 悪い方の直線が良くなりうるのは、左端で良かったなら [l, m)、そうでなければ [m, r)
            if (l_better != m_better) {
                k = child(k, false), r = m;
            } else {
                k = child(k, true), l = m;
            }
        }
    }
    // 頂点 k (区間 [l, r)、[sl, sr) と重なる) に線分を入れる。重ならない側の子は作らない
    void update_segment(int k, long long l, long long r, T a, T b, long long sl, long long sr) {
        if (sl <= l && r <= sr) {
            update(k, l, r, a, b);
            return;
        }
        const long long m = l + (r - l) / 2;
        if (sl < m) update_segment(child(k, false), l, m, a, b, sl, sr);
        if (m < sr) update_segment(child(k, true), m, r, a, b, sl, sr);
    }

   public:
    /**
     * @brief x が [lo, hi) の整数をとる、直線のない木を作る
     */
    DynamicLiChaoTree(long long lo, long long hi) : lo(lo), hi(hi), nodes(1) { assert(lo < hi); }

    /**
     * @brief 直線 y = a x + b を追加する O(log (hi - lo))
     */
    void add_line(T a, T b) { update(0, lo, hi, a, b); }
    /**
     * @brief 線分 y = a x + b (x ∈ [l, r)) を追加する O(log^2 (hi - lo))
     */
    void add_segment(T a, T b, long long l, long long r) {
        assert(lo <= l && l <= r && r <= hi);
        if (l < r) update_segment(0, lo, hi, a, b, l, r);
    }
    /**
     * @brief x での最小値。x を含む直線・線分がなければ std::nullopt O(log (hi - lo))
     */
    std::optional<T> get_min(long long x) const {
        assert(lo <= x && x < hi);
        std::optional<T> res;
        long long l = lo, r = hi;
        for (int k = 0; k != -1;) {
            const Node& nd = nodes[k];
            if (nd.exist) {
                T v = eval(nd.a, nd.b, x);
                if (!res || v < *res) res = v;
            }
            const long long m = l + (r - l) / 2;
            if (x < m) {
                k = nd.left, r = m;
            } else {
                k = nd.right, l = m;
            }
        }
        return res;
    }
};
