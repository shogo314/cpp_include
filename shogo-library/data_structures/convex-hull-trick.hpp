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
// https://github.com/shogo314/shogo-library/blob/b591e1ab6db09d0025e385b1bb5f4d8982cc4d1f/src/data_structures/convex-hull-trick.hpp

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
        while (sz < (long long)xs.size()) {
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
