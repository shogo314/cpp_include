#pragma once

/**
 * @file dynamic-segtree.hpp
 * @brief 動的セグメント木
 */

#include <cassert>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/cfbc779abd4bff3efa5b12de9b69f0aabc01392d/src/data_structures/dynamic-segtree.hpp

/**
 * @brief 動的セグメント木
 *
 * 要素数が大きくても使えるセグメント木。最初はすべて単位元で、更新したところのノードだけを作る
 * 例:
 * DynamicSegTree<monoid::Sum<long long>> seg(1000000000000000000);
 * seg.set(123456789012, 5);
 * seg.prod(0, 1000000000000000000);  // 5
 *
 * @tparam M モノイド (algebra/monoid.hpp)
 */
template <typename M>
class DynamicSegTree {
   public:
    using S = typename M::S;

   private:
    M m;
    S op(const S& a, const S& b) const { return m.op(a, b); }
    S e() const { return m.e(); }

    long long n;
    int root;  // まだ作っていないノードは -1 で、区間のすべての要素が単位元であることを表す
    std::vector<S> val;
    std::vector<int> left, right;

    S value(int t) const { return t == -1 ? e() : val[t]; }
    int make_node() {
        val.push_back(e());
        left.push_back(-1);
        right.push_back(-1);
        return int(val.size()) - 1;
    }

    class DynamicSegTreeReference {
        DynamicSegTree& segtree;
        long long k;

       public:
        DynamicSegTreeReference(DynamicSegTree& segtree, long long k) : segtree(segtree), k(k) {}
        DynamicSegTreeReference& operator=(const S& x) {
            segtree.set(k, x);
            return *this;
        }
        operator S() const { return segtree.get(k); }
    };

    int set_rec(int t, long long a, long long b, long long k, const S& x) {
        if (t == -1) t = make_node();
        if (b - a == 1) {
            val[t] = x;
            return t;
        }
        long long mid = (a + b) / 2;
        if (k < mid) {
            int c = set_rec(left[t], a, mid, k, x);
            left[t] = c;
        } else {
            int c = set_rec(right[t], mid, b, k, x);
            right[t] = c;
        }
        val[t] = op(value(left[t]), value(right[t]));
        return t;
    }
    S prod_rec(int t, long long a, long long b, long long l, long long r) const {
        if (t == -1 || r <= a || b <= l) return e();
        if (l <= a && b <= r) return val[t];
        long long mid = (a + b) / 2;
        return op(prod_rec(left[t], a, mid, l, r), prod_rec(right[t], mid, b, l, r));
    }
    // [a, b) のうち l 以降で、sum に掛けると f が偽になる最初の位置 (なければ b)
    template <typename F>
    long long max_right_rec(int t, long long a, long long b, long long l, F& f, S& sum) const {
        // 作っていないノードは単位元だけなので、f は偽にならない
        if (t == -1 || b <= l) return b;
        if (l <= a) {
            S s = op(sum, val[t]);
            if (f(s)) {
                sum = s;
                return b;
            }
            if (b - a == 1) return a;
        }
        long long mid = (a + b) / 2;
        long long x = max_right_rec(left[t], a, mid, l, f, sum);
        if (x < mid) return x;
        return max_right_rec(right[t], mid, b, l, f, sum);
    }
    // [a, b) のうち r より前で、sum に左から掛けると f が偽になる最後の位置 + 1 (なければ a)
    template <typename F>
    long long min_left_rec(int t, long long a, long long b, long long r, F& f, S& sum) const {
        if (t == -1 || r <= a) return a;
        if (b <= r) {
            S s = op(val[t], sum);
            if (f(s)) {
                sum = s;
                return a;
            }
            if (b - a == 1) return b;
        }
        long long mid = (a + b) / 2;
        long long x = min_left_rec(right[t], mid, b, r, f, sum);
        if (x > mid) return x;
        return min_left_rec(left[t], a, mid, r, f, sum);
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * すべての要素を単位元で初期化する。ノードは更新したときに作るので O(1)
     *
     * @param n 要素数
     * @param m モノイド
     */
    explicit DynamicSegTree(long long n = 0, M m = M()) : m(std::move(m)), n(n), root(-1) {
        assert(0 <= n);
    }

    /**
     * @brief 要素数を返す
     */
    long long size() const { return n; }

    /**
     * @brief 指定された要素の値を返す
     *
     * @param k インデックス
     * @return S 値
     */
    S get(long long k) const {
        assert(0 <= k && k < n);
        int t = root;
        long long a = 0, b = n;
        while (t != -1 && b - a > 1) {
            long long mid = (a + b) / 2;
            if (k < mid) {
                t = left[t];
                b = mid;
            } else {
                t = right[t];
                a = mid;
            }
        }
        return value(t);
    }
    /**
     * @brief 指定された要素の値を返す
     *
     * @param k インデックス
     * @return S 値
     */
    S operator[](long long k) const { return get(k); }
    /**
     * @brief 指定された要素への参照を返す
     *
     * @param k
     * @return DynamicSegTreeReference 要素への参照 代入されるとset()が呼ばれる
     */
    DynamicSegTreeReference operator[](long long k) { return DynamicSegTreeReference(*this, k); }

    /**
     * @brief 指定された要素の値をxに更新する
     *
     * @param k インデックス
     * @param x 新しい値
     */
    void set(long long k, const S& x) {
        assert(0 <= k && k < n);
        root = set_rec(root, 0, n, k, x);
    }
    /**
     * @brief 指定された要素の値にxを作用させる
     *
     * @param k インデックス
     * @param x 作用素
     */
    void apply(long long k, const S& x) { set(k, op(get(k), x)); }

    /**
     * @brief [l, r)の区間の総積を返す
     *
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @return S 総積
     */
    S prod(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        return prod_rec(root, 0, n, l, r);
    }
    /**
     * @brief すべての要素の総積を返す
     *
     * @return S 総積
     */
    S all_prod() const { return value(root); }

    /**
     * @brief (r = l or f(prod([l, r))) = true) and (r = n or f(prod([l, r+1))) = false)となるrを返す
     * fが単調なら、f(prod([l, r))) = trueとなる最大のr
     *
     * @tparam F
     * @param l 半開区間の開始 0<=l<=n
     * @param f 判定関数 f(e) = true
     * @return long long
     */
    template <typename F>
    long long max_right(long long l, F f) const {
        assert(f(e()));
        assert(0 <= l && l <= n);
        if (l == n) return n;
        S sum = e();
        return max_right_rec(root, 0, n, l, f, sum);
    }
    /**
     * @brief (l = 0 or f(prod([l, r))) = true) and (l = r or f(prod([l-1, r))) = false)となるlを返す
     * fが単調なら、f(prod([l, r))) = trueとなる最小のl
     *
     * @tparam F
     * @param r 半開区間の終端 0<=r<=n
     * @param f 判定関数 f(e) = true
     * @return long long
     */
    template <typename F>
    long long min_left(long long r, F f) const {
        assert(f(e()));
        assert(0 <= r && r <= n);
        if (r == 0) return 0;
        S sum = e();
        return min_left_rec(root, 0, n, r, f, sum);
    }
};

template <typename M>
DynamicSegTree(long long, M) -> DynamicSegTree<M>;
