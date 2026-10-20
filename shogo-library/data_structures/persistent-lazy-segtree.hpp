#pragma once

/**
 * @file persistent-lazy-segtree.hpp
 * @brief 永続遅延伝搬セグメント木
 */

#include <algorithm>
#include <cassert>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include "../algebra/acted-monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/data_structures/persistent-lazy-segtree.hpp

/**
 * @brief 永続遅延伝搬セグメント木
 *
 * 更新すると新しい版を返し、元の版は変わらない。同じ木から作った版はノードを共有する
 * 別の版の区間をそのまま写した版も作れる
 * 例:
 * PersistentLazySegTree<acted_monoid::AffineSum<mint>> s0(v);
 * auto s1 = s0.apply(l, r, Affine<mint>{b, c});  // s0 はそのまま
 * auto s2 = s1.copy(s0, l, r);                   // [l, r) を s0 の値に戻した版
 *
 * @tparam A 作用素付きモノイド (algebra/acted-monoid.hpp)
 */
template <typename A>
class PersistentLazySegTree {
   public:
    using S = typename A::S;
    using F = typename A::F;

   private:
    // すべての版のノードを持つ。ノードは削除しない
    // val はノードの区間の積で、そのノードの lazy は適用済み。lazy は子にまだ伝えていない作用素
    struct Pool {
        A a;
        std::vector<S> val;
        std::vector<F> lazy;
        std::vector<int> left, right;  // 葉では -1
        explicit Pool(A a) : a(std::move(a)) {}
        int make_leaf(const S& x) {
            val.push_back(x);
            lazy.push_back(a.id());
            left.push_back(-1);
            right.push_back(-1);
            return static_cast<int>(val.size()) - 1;
        }
        int make_node(int l, int r) {
            val.push_back(a.op(val[l], val[r]));
            lazy.push_back(a.id());
            left.push_back(l);
            right.push_back(r);
            return static_cast<int>(val.size()) - 1;
        }
    };

    std::shared_ptr<Pool> pool;
    long long n, sz;
    int root;

    S op(const S& x, const S& y) const { return pool->a.op(x, y); }
    S e() const { return pool->a.e(); }
    // 末尾の要素数を 2 べきにそろえるための余りの部分は区間に含めない
    S mapping(const F& f, const S& x, long long a, long long b) const { return acted_monoid::apply_mapping(pool->a, f, x, std::min(a, n), std::min(b, n)); }
    F composition(const F& f, const F& g) const { return pool->a.composition(f, g); }
    F id() const { return pool->a.id(); }

    PersistentLazySegTree(std::shared_ptr<Pool> pool, long long n, long long sz, int root) : pool(std::move(pool)), n(n), sz(sz), root(root) {}

    void init_size() {
        sz = 1;
        while (sz < n) sz <<= 1;
    }
    int build(const std::vector<S>& v, long long a, long long b) {
        if (b - a == 1) return pool->make_leaf(a < n ? v[a] : e());
        long long mid = (a + b) / 2;
        int l = build(v, a, mid);
        int r = build(v, mid, b);
        return pool->make_node(l, r);
    }
    // ノード t ([a, b)) に f を作用させた新しいノード
    int apply_node(int t, long long a, long long b, const F& f) const {
        S x = mapping(f, pool->val[t], a, b);
        if (b - a == 1) return pool->make_leaf(x);
        F g = composition(f, pool->lazy[t]);
        int l = pool->left[t], r = pool->right[t];
        pool->val.push_back(x);
        pool->lazy.push_back(g);
        pool->left.push_back(l);
        pool->right.push_back(r);
        return static_cast<int>(pool->val.size()) - 1;
    }
    // ノード t ([a, b)) の lazy を子に伝えた、新しい子の組
    std::pair<int, int> push(int t, long long a, long long b) const {
        long long mid = (a + b) / 2;
        const F f = pool->lazy[t];
        int l = apply_node(pool->left[t], a, mid, f);
        int r = apply_node(pool->right[t], mid, b, f);
        return {l, r};
    }
    int set_rec(int t, long long a, long long b, long long k, const S& x) const {
        if (b - a == 1) return pool->make_leaf(x);
        long long mid = (a + b) / 2;
        auto [l, r] = push(t, a, b);
        if (k < mid)
            l = set_rec(l, a, mid, k, x);
        else
            r = set_rec(r, mid, b, k, x);
        return pool->make_node(l, r);
    }
    int apply_rec(int t, long long a, long long b, long long l, long long r, const F& f) const {
        if (r <= a || b <= l) return t;
        if (l <= a && b <= r) return apply_node(t, a, b, f);
        long long mid = (a + b) / 2;
        auto [lc, rc] = push(t, a, b);
        lc = apply_rec(lc, a, mid, l, r, f);
        rc = apply_rec(rc, mid, b, l, r, f);
        return pool->make_node(lc, rc);
    }
    // t の [l, r) を u に置き換える。f は u の祖先にあって、まだ u に適用していない作用素
    int copy_rec(int t, int u, long long a, long long b, long long l, long long r, const F& f) const {
        if (r <= a || b <= l) return t;
        if (l <= a && b <= r) return apply_node(u, a, b, f);
        long long mid = (a + b) / 2;
        auto [lc, rc] = push(t, a, b);
        F g = composition(f, pool->lazy[u]);
        lc = copy_rec(lc, pool->left[u], a, mid, l, r, g);
        rc = copy_rec(rc, pool->right[u], mid, b, l, r, g);
        return pool->make_node(lc, rc);
    }
    // f はノード t の祖先にあって、まだ t に適用していない作用素
    S prod_rec(int t, long long a, long long b, long long l, long long r, const F& f) const {
        if (r <= a || b <= l) return e();
        if (l <= a && b <= r) return mapping(f, pool->val[t], a, b);
        long long mid = (a + b) / 2;
        F g = composition(f, pool->lazy[t]);
        return op(prod_rec(pool->left[t], a, mid, l, r, g), prod_rec(pool->right[t], mid, b, l, r, g));
    }
    // [a, b) のうち l 以降で、sum に掛けると g が偽になる最初の位置 (なければ b)
    template <typename G>
    long long max_right_rec(int t, long long a, long long b, long long l, G& g, S& sum, const F& f) const {
        if (b <= l) return b;
        if (l <= a) {
            S s = op(sum, mapping(f, pool->val[t], a, b));
            if (g(s)) {
                sum = s;
                return b;
            }
            if (b - a == 1) return a;
        }
        long long mid = (a + b) / 2;
        F h = composition(f, pool->lazy[t]);
        long long x = max_right_rec(pool->left[t], a, mid, l, g, sum, h);
        if (x < mid) return x;
        return max_right_rec(pool->right[t], mid, b, l, g, sum, h);
    }
    // [a, b) のうち r より前で、sum に左から掛けると g が偽になる最後の位置 + 1 (なければ a)
    template <typename G>
    long long min_left_rec(int t, long long a, long long b, long long r, G& g, S& sum, const F& f) const {
        if (r <= a) return a;
        if (b <= r) {
            S s = op(mapping(f, pool->val[t], a, b), sum);
            if (g(s)) {
                sum = s;
                return a;
            }
            if (b - a == 1) return b;
        }
        long long mid = (a + b) / 2;
        F h = composition(f, pool->lazy[t]);
        long long x = min_left_rec(pool->right[t], mid, b, r, g, sum, h);
        if (x > mid) return x;
        return min_left_rec(pool->left[t], a, mid, r, g, sum, h);
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * すべての要素を単位元で初期化する。単位元の部分木は共有するので、n が大きくても O(log n) で作れる
     *
     * @param n 要素数
     * @param a 作用素付きモノイド
     */
    explicit PersistentLazySegTree(long long n = 0, A a = A()) : pool(std::make_shared<Pool>(std::move(a))), n(n) {
        assert(0 <= n);
        init_size();
        root = pool->make_leaf(e());
        for (long long w = 1; w < sz; w <<= 1) root = pool->make_node(root, root);
    }
    /**
     * @brief コンストラクタ
     *
     * @param v 初期の要素
     * @param a 作用素付きモノイド
     */
    explicit PersistentLazySegTree(const std::vector<S>& v, A a = A()) : pool(std::make_shared<Pool>(std::move(a))), n(v.size()) {
        init_size();
        root = build(v, 0, sz);
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
        long long a = 0, b = sz;
        F f = id();
        while (b - a > 1) {
            f = composition(f, pool->lazy[t]);
            long long mid = (a + b) / 2;
            if (k < mid) {
                t = pool->left[t];
                b = mid;
            } else {
                t = pool->right[t];
                a = mid;
            }
        }
        return mapping(f, pool->val[t], a, b);
    }
    /**
     * @brief 指定された要素の値を返す
     *
     * @param k インデックス
     * @return S 値
     */
    S operator[](long long k) const { return get(k); }

    /**
     * @brief 内容を出力する
     *
     * @tparam CharT 出力ストリームの文字型
     * @tparam Traits 出力ストリームの文字型特性
     * @param os 出力ストリーム
     * @param rhs 永続遅延伝搬セグメント木
     * @return std::basic_ostream<CharT, Traits>& 出力ストリーム
     */
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const PersistentLazySegTree& rhs) {
        for (long long i = 0; i < rhs.n; i++) {
            if (i != 0) os << CharT(' ');
            os << rhs[i];
        }
        return os;
    }

    /**
     * @brief 指定された要素の値をxにした新しい版を返す
     *
     * @param k インデックス
     * @param x 新しい値
     * @return PersistentLazySegTree 新しい版 (自身は変わらない)
     */
    PersistentLazySegTree set(long long k, const S& x) const {
        assert(0 <= k && k < n);
        return PersistentLazySegTree(pool, n, sz, set_rec(root, 0, sz, k, x));
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
        return prod_rec(root, 0, sz, l, r, id());
    }
    /**
     * @brief すべての要素の総積を返す
     *
     * @return S 総積
     */
    S all_prod() const { return pool->val[root]; }

    /**
     * @brief 指定された要素の値にfを作用させた新しい版を返す
     *
     * @param k インデックス
     * @param f 作用素
     * @return PersistentLazySegTree 新しい版 (自身は変わらない)
     */
    PersistentLazySegTree apply(long long k, const F& f) const {
        assert(0 <= k && k < n);
        return apply(k, k + 1, f);
    }
    /**
     * @brief [l, r)の区間の値にfを作用させた新しい版を返す
     *
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @param f 作用素
     * @return PersistentLazySegTree 新しい版 (自身は変わらない)
     */
    PersistentLazySegTree apply(long long l, long long r, const F& f) const {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return *this;
        return PersistentLazySegTree(pool, n, sz, apply_rec(root, 0, sz, l, r, f));
    }

    /**
     * @brief [l, r)の区間を別の版の値にした新しい版を返す
     *
     * @param other 写す元の版 (自身と同じ木から作ったもの)
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @return PersistentLazySegTree 新しい版 (自身も other も変わらない)
     */
    PersistentLazySegTree copy(const PersistentLazySegTree& other, long long l, long long r) const {
        assert(pool == other.pool);
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return *this;
        return PersistentLazySegTree(pool, n, sz, copy_rec(root, other.root, 0, sz, l, r, id()));
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
        long long r = max_right_rec(root, 0, sz, l, g, sum, id());
        return r < n ? r : n;
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
        return min_left_rec(root, 0, sz, r, g, sum, id());
    }

    /**
     * @brief すべての要素を並べた列を返す
     */
    std::vector<S> to_vector() const {
        std::vector<S> res(n);
        for (long long i = 0; i < n; i++) res[i] = get(i);
        return res;
    }
};

template <typename A>
PersistentLazySegTree(long long, A) -> PersistentLazySegTree<A>;
template <typename A>
PersistentLazySegTree(std::vector<typename A::S>, A) -> PersistentLazySegTree<A>;
