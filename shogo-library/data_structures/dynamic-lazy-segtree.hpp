#pragma once

/**
 * @file dynamic-lazy-segtree.hpp
 * @brief 動的遅延伝搬セグメント木
 */

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "../algebra/acted-monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/cfbc779abd4bff3efa5b12de9b69f0aabc01392d/src/data_structures/dynamic-lazy-segtree.hpp

/**
 * @brief 動的遅延伝搬セグメント木
 *
 * 要素数が大きくても使える遅延伝搬セグメント木。最初はすべて単位元で、更新したところのノードだけを作る
 * 例:
 * DynamicLazySegTree<acted_monoid::AddSum<long long>> seg(1000000000000000000);
 * seg.apply(0, 1000000000000000000, 1);
 * seg.prod(10, 20);  // 10
 *
 * @tparam A 作用素付きモノイド (algebra/acted-monoid.hpp)
 */
template <typename A>
class DynamicLazySegTree {
   public:
    using S = typename A::S;
    using F = typename A::F;

   private:
    A a;
    S op(const S& x, const S& y) const { return a.op(x, y); }
    S e() const { return a.e(); }
    S mapping(const F& f, const S& x, long long l, long long r) const { return acted_monoid::apply_mapping(a, f, x, l, r); }
    F composition(const F& f, const F& g) const { return a.composition(f, g); }
    F id() const { return a.id(); }

    long long n;
    int root;  // まだ作っていないノードは -1 で、区間のすべての要素が単位元であることを表す
    // val はノードの区間の積で、そのノードの lazy は適用済み。lazy は子にまだ伝えていない作用素
    std::vector<S> val;
    std::vector<F> lazy;
    std::vector<int> left, right;

    class DynamicLazySegTreeReference {
        DynamicLazySegTree& segtree;
        long long k;

       public:
        DynamicLazySegTreeReference(DynamicLazySegTree& segtree, long long k) : segtree(segtree), k(k) {}
        DynamicLazySegTreeReference& operator=(const S& x) {
            segtree.set(k, x);
            return *this;
        }
        operator S() const { return segtree.get(k); }
    };

    int make_node(const S& x, const F& f) {
        val.push_back(x);
        lazy.push_back(f);
        left.push_back(-1);
        right.push_back(-1);
        return int(val.size()) - 1;
    }
    // ノード t ([a, b)) に f を作用させる。t が -1 ならノードを作る
    int apply_node(int t, long long a, long long b, const F& f) {
        if (t == -1) return make_node(mapping(f, e(), a, b), f);
        val[t] = mapping(f, val[t], a, b);
        lazy[t] = composition(f, lazy[t]);
        return t;
    }
    // ノード t ([a, b)) の lazy を子に伝える
    void push(int t, long long a, long long b) {
        long long mid = (a + b) / 2;
        const F f = lazy[t];
        int l = apply_node(left[t], a, mid, f);
        left[t] = l;
        int r = apply_node(right[t], mid, b, f);
        right[t] = r;
        lazy[t] = id();
    }
    S value(int t) const { return t == -1 ? e() : val[t]; }
    void update(int t) { val[t] = op(value(left[t]), value(right[t])); }

    int set_rec(int t, long long a, long long b, long long k, const S& x) {
        if (b - a == 1) {
            if (t == -1) return make_node(x, id());
            val[t] = x;
            return t;
        }
        long long mid = (a + b) / 2;
        // 作ったばかりのノードは子がなく lazy も恒等写像なので、伝えなくてよい
        if (t == -1)
            t = make_node(e(), id());
        else
            push(t, a, b);
        if (k < mid) {
            int c = set_rec(left[t], a, mid, k, x);
            left[t] = c;
        } else {
            int c = set_rec(right[t], mid, b, k, x);
            right[t] = c;
        }
        update(t);
        return t;
    }
    int apply_rec(int t, long long a, long long b, long long l, long long r, const F& f) {
        if (r <= a || b <= l) return t;
        if (l <= a && b <= r) return apply_node(t, a, b, f);
        long long mid = (a + b) / 2;
        if (t == -1)
            t = make_node(e(), id());
        else
            push(t, a, b);
        int lc = apply_rec(left[t], a, mid, l, r, f);
        left[t] = lc;
        int rc = apply_rec(right[t], mid, b, l, r, f);
        right[t] = rc;
        update(t);
        return t;
    }
    // f はノード t の祖先にあって、まだ t に適用していない作用素
    S prod_rec(int t, long long a, long long b, long long l, long long r, const F& f) const {
        if (r <= a || b <= l) return e();
        // 作っていないノードの区間は単位元だけなので、f を作用させた値は区間の位置だけで決まる
        if (t == -1) return mapping(f, e(), std::max(a, l), std::min(b, r));
        if (l <= a && b <= r) return mapping(f, val[t], a, b);
        long long mid = (a + b) / 2;
        F g = composition(f, lazy[t]);
        return op(prod_rec(left[t], a, mid, l, r, g), prod_rec(right[t], mid, b, l, r, g));
    }
    // [a, b) のうち l 以降で、sum に掛けると g が偽になる最初の位置 (なければ b)
    template <typename G>
    long long max_right_rec(int t, long long a, long long b, long long l, G& g, S& sum, const F& f) const {
        if (b <= l) return b;
        if (l <= a) {
            S s = op(sum, t == -1 ? mapping(f, e(), a, b) : mapping(f, val[t], a, b));
            if (g(s)) {
                sum = s;
                return b;
            }
            if (b - a == 1) return a;
        }
        long long mid = (a + b) / 2;
        F h = t == -1 ? f : composition(f, lazy[t]);
        long long x = max_right_rec(t == -1 ? -1 : left[t], a, mid, l, g, sum, h);
        if (x < mid) return x;
        return max_right_rec(t == -1 ? -1 : right[t], mid, b, l, g, sum, h);
    }
    // [a, b) のうち r より前で、sum に左から掛けると g が偽になる最後の位置 + 1 (なければ a)
    template <typename G>
    long long min_left_rec(int t, long long a, long long b, long long r, G& g, S& sum, const F& f) const {
        if (r <= a) return a;
        if (b <= r) {
            S s = op(t == -1 ? mapping(f, e(), a, b) : mapping(f, val[t], a, b), sum);
            if (g(s)) {
                sum = s;
                return a;
            }
            if (b - a == 1) return b;
        }
        long long mid = (a + b) / 2;
        F h = t == -1 ? f : composition(f, lazy[t]);
        long long x = min_left_rec(t == -1 ? -1 : right[t], mid, b, r, g, sum, h);
        if (x > mid) return x;
        return min_left_rec(t == -1 ? -1 : left[t], a, mid, r, g, sum, h);
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * すべての要素を単位元で初期化する。ノードは更新したときに作るので O(1)
     *
     * @param n 要素数
     * @param a 作用素付きモノイド
     */
    explicit DynamicLazySegTree(long long n = 0, A a = A()) : a(std::move(a)), n(n), root(-1) {
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
        F f = id();
        while (t != -1 && b - a > 1) {
            f = composition(f, lazy[t]);
            long long mid = (a + b) / 2;
            if (k < mid) {
                t = left[t];
                b = mid;
            } else {
                t = right[t];
                a = mid;
            }
        }
        return mapping(f, t == -1 ? e() : val[t], k, k + 1);
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
     * @return DynamicLazySegTreeReference 要素への参照 代入されるとset()が呼ばれる
     */
    DynamicLazySegTreeReference operator[](long long k) { return DynamicLazySegTreeReference(*this, k); }

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
     * @brief [l, r)の区間の総積を返す
     *
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @return S 総積
     */
    S prod(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return e();
        return prod_rec(root, 0, n, l, r, id());
    }
    /**
     * @brief すべての要素の総積を返す
     *
     * @return S 総積
     */
    S all_prod() const { return value(root); }

    /**
     * @brief 指定された要素の値にfを作用させる
     *
     * @param k インデックス
     * @param f 作用素
     */
    void apply(long long k, const F& f) {
        assert(0 <= k && k < n);
        apply(k, k + 1, f);
    }
    /**
     * @brief [l, r)の区間の値にfを作用させる
     *
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @param f 作用素
     */
    void apply(long long l, long long r, const F& f) {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return;
        root = apply_rec(root, 0, n, l, r, f);
    }

    /**
     * @brief (r = l or g(prod([l, r))) = true) and (r = n or g(prod([l, r+1))) = false)となるrを返す
     * gが単調なら、g(prod([l, r))) = trueとなる最大のr
     *
     * @tparam G
     * @param l 半開区間の開始 0<=l<=n
     * @param g 判定関数 g(e) = true
     * @return long long
     */
    template <typename G>
    long long max_right(long long l, G g) const {
        assert(g(e()));
        assert(0 <= l && l <= n);
        if (l == n) return n;
        S sum = e();
        return max_right_rec(root, 0, n, l, g, sum, id());
    }
    /**
     * @brief (l = 0 or g(prod([l, r))) = true) and (l = r or g(prod([l-1, r))) = false)となるlを返す
     * gが単調なら、g(prod([l, r))) = trueとなる最小のl
     *
     * @tparam G
     * @param r 半開区間の終端 0<=r<=n
     * @param g 判定関数 g(e) = true
     * @return long long
     */
    template <typename G>
    long long min_left(long long r, G g) const {
        assert(g(e()));
        assert(0 <= r && r <= n);
        if (r == 0) return 0;
        S sum = e();
        return min_left_rec(root, 0, n, r, g, sum, id());
    }
};

template <typename A>
DynamicLazySegTree(long long, A) -> DynamicLazySegTree<A>;
