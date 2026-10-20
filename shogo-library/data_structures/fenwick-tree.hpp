#pragma once

/**
 * @file fenwick-tree.hpp
 * @brief Fenwick Tree (Binary Indexed Tree)
 */

#include <cassert>
#include <utility>
#include <vector>

#include "../algebra/group.hpp"
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/data_structures/fenwick-tree.hpp

/**
 * @brief Fenwick Tree (Binary Indexed Tree)
 *
 * 1 点への作用と区間積を O(log n) で行う
 * 例:
 * FenwickTree<group::Sum<long long>> ft(n);
 * ft.apply(i, x);  // a[i] += x
 * ft.prod(l, r);   // a[l] + ... + a[r-1]
 *
 * @tparam G 可換な群 (algebra/group.hpp)。prefix_prod と max_right だけなら可換モノイドでもよい
 */
template <typename G>
class FenwickTree {
   public:
    using S = typename G::S;

   private:
    G g;
    long long n;
    // data[i] は a[i - (i & -i)] から a[i-1] までの積 (1-indexed)
    std::vector<S> data;
    S op(const S& a, const S& b) const { return g.op(a, b); }
    S e() const { return g.e(); }
    S inv(const S& a) const { return g.inv(a); }

   public:
    /**
     * @brief 要素数 n で、すべての要素が単位元の Fenwick Tree を作る
     */
    explicit FenwickTree(long long n = 0, G g = G()) : g(std::move(g)), n(n), data(n + 1, e()) {}
    /**
     * @brief v を初期値とする Fenwick Tree を作る O(n)
     */
    explicit FenwickTree(const std::vector<S>& v, G g = G()) : FenwickTree(v.size(), std::move(g)) {
        for (long long i = 1; i <= n; i++) {
            data[i] = op(data[i], v[i - 1]);
            long long j = i + (i & -i);
            if (j <= n) data[j] = op(data[j], data[i]);
        }
    }

    long long size() const { return n; }

    /**
     * @brief a[k] を op(a[k], x) にする O(log n)
     */
    void apply(long long k, const S& x) {
        assert(0 <= k && k < n);
        for (k++; k <= n; k += k & -k) data[k] = op(data[k], x);
    }
    /**
     * @brief a[0] から a[r-1] までの積 O(log n)
     */
    S prefix_prod(long long r) const {
        assert(0 <= r && r <= n);
        S res = e();
        for (; r > 0; r -= r & -r) res = op(data[r], res);
        return res;
    }
    /**
     * @brief a[l] から a[r-1] までの積 O(log n)。逆元を使う
     */
    S prod(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        return op(inv(prefix_prod(l)), prefix_prod(r));
    }
    S all_prod() const { return prefix_prod(n); }
    /**
     * @brief a[k] O(log n)。逆元を使う
     */
    S get(long long k) const {
        assert(0 <= k && k < n);
        return prod(k, k + 1);
    }
    /**
     * @brief a[k] を x にする O(log n)。逆元を使う
     */
    void set(long long k, const S& x) { apply(k, op(inv(get(k)), x)); }

    /**
     * @brief f(prefix_prod(r)) = true となる最大の r を返す O(log n)
     * f は単調 (ある r までは true でそれ以降は false) であること。f(e()) = true
     */
    template <typename F>
    long long max_right(F f) const {
        assert(f(e()));
        long long r = 0;
        S sum = e();
        long long step = 1;
        while (step * 2 <= n) step *= 2;
        for (; step > 0; step /= 2) {
            if (r + step <= n && f(op(sum, data[r + step]))) {
                r += step;
                sum = op(sum, data[r]);
            }
        }
        return r;
    }
};

template <typename G>
FenwickTree(long long, G) -> FenwickTree<G>;
template <typename G>
FenwickTree(std::vector<typename G::S>, G) -> FenwickTree<G>;
