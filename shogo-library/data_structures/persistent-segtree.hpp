#pragma once

/**
 * @file persistent-segtree.hpp
 * @brief 永続セグメント木
 */

#include <cassert>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/24638d870d2ac03d01e1498411ab8aa477ea366f/src/data_structures/persistent-segtree.hpp

/**
 * @brief 永続セグメント木
 *
 * 更新すると新しい版を返し、元の版は変わらない。同じ木から作った版はノードを共有する
 * 例:
 * PersistentSegTree<monoid::Sum<long long>> s0(n);
 * auto s1 = s0.set(3, 10);  // s0 はそのまま
 * s1.prod(0, 5);            // 10
 *
 * @tparam M モノイド (algebra/monoid.hpp)
 */
template <typename M>
class PersistentSegTree {
   public:
    using S = typename M::S;

   private:
    // すべての版のノードを持つ。ノードは削除しない
    struct Pool {
        M m;
        std::vector<S> val;
        std::vector<int> left, right;  // 葉では -1
        explicit Pool(M m) : m(std::move(m)) {}
        int make_leaf(const S& x) {
            val.push_back(x);
            left.push_back(-1);
            right.push_back(-1);
            return int(val.size()) - 1;
        }
        int make_node(int l, int r) {
            val.push_back(m.op(val[l], val[r]));
            left.push_back(l);
            right.push_back(r);
            return int(val.size()) - 1;
        }
    };

    std::shared_ptr<Pool> pool;
    long long n, sz;
    int root;

    S op(const S& a, const S& b) const { return pool->m.op(a, b); }
    S e() const { return pool->m.e(); }

    PersistentSegTree(std::shared_ptr<Pool> pool, long long n, long long sz, int root) : pool(std::move(pool)), n(n), sz(sz), root(root) {}

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
    int set_rec(int t, long long a, long long b, long long k, const S& x) const {
        if (b - a == 1) return pool->make_leaf(x);
        long long mid = (a + b) / 2;
        int l = pool->left[t], r = pool->right[t];
        if (k < mid)
            l = set_rec(l, a, mid, k, x);
        else
            r = set_rec(r, mid, b, k, x);
        return pool->make_node(l, r);
    }
    // t の [l, r) を u に置き換える
    int copy_rec(int t, int u, long long a, long long b, long long l, long long r) const {
        if (r <= a || b <= l) return t;
        if (l <= a && b <= r) return u;
        long long mid = (a + b) / 2;
        int lc = copy_rec(pool->left[t], pool->left[u], a, mid, l, r);
        int rc = copy_rec(pool->right[t], pool->right[u], mid, b, l, r);
        return pool->make_node(lc, rc);
    }
    S prod_rec(int t, long long a, long long b, long long l, long long r) const {
        if (r <= a || b <= l) return e();
        if (l <= a && b <= r) return pool->val[t];
        long long mid = (a + b) / 2;
        return op(prod_rec(pool->left[t], a, mid, l, r), prod_rec(pool->right[t], mid, b, l, r));
    }
    // [a, b) のうち l 以降で、sum に掛けると f が偽になる最初の位置 (なければ b)
    template <typename F>
    long long max_right_rec(int t, long long a, long long b, long long l, F& f, S& sum) const {
        if (b <= l) return b;
        if (l <= a) {
            S s = op(sum, pool->val[t]);
            if (f(s)) {
                sum = s;
                return b;
            }
            if (b - a == 1) return a;
        }
        long long mid = (a + b) / 2;
        long long x = max_right_rec(pool->left[t], a, mid, l, f, sum);
        if (x < mid) return x;
        return max_right_rec(pool->right[t], mid, b, l, f, sum);
    }
    // [a, b) のうち r より前で、sum に左から掛けると f が偽になる最後の位置 + 1 (なければ a)
    template <typename F>
    long long min_left_rec(int t, long long a, long long b, long long r, F& f, S& sum) const {
        if (r <= a) return a;
        if (b <= r) {
            S s = op(pool->val[t], sum);
            if (f(s)) {
                sum = s;
                return a;
            }
            if (b - a == 1) return b;
        }
        long long mid = (a + b) / 2;
        long long x = min_left_rec(pool->right[t], mid, b, r, f, sum);
        if (x > mid) return x;
        return min_left_rec(pool->left[t], a, mid, r, f, sum);
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * すべての要素を単位元で初期化する。単位元の部分木は共有するので、n が大きくても O(log n) で作れる
     *
     * @param n 要素数
     * @param m モノイド
     */
    explicit PersistentSegTree(long long n = 0, M m = M()) : pool(std::make_shared<Pool>(std::move(m))), n(n) {
        assert(0 <= n);
        init_size();
        root = pool->make_leaf(e());
        for (long long w = 1; w < sz; w <<= 1) root = pool->make_node(root, root);
    }
    /**
     * @brief コンストラクタ
     *
     * @param v 初期の要素
     * @param m モノイド
     */
    explicit PersistentSegTree(const std::vector<S>& v, M m = M()) : pool(std::make_shared<Pool>(std::move(m))), n(v.size()) {
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
        while (b - a > 1) {
            long long mid = (a + b) / 2;
            if (k < mid) {
                t = pool->left[t];
                b = mid;
            } else {
                t = pool->right[t];
                a = mid;
            }
        }
        return pool->val[t];
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
     * @param rhs 永続セグメント木
     * @return std::basic_ostream<CharT, Traits>& 出力ストリーム
     */
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const PersistentSegTree& rhs) {
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
     * @return PersistentSegTree 新しい版 (自身は変わらない)
     */
    PersistentSegTree set(long long k, const S& x) const {
        assert(0 <= k && k < n);
        return PersistentSegTree(pool, n, sz, set_rec(root, 0, sz, k, x));
    }
    /**
     * @brief 指定された要素の値にxを作用させた新しい版を返す
     *
     * @param k インデックス
     * @param x 作用素
     * @return PersistentSegTree 新しい版 (自身は変わらない)
     */
    PersistentSegTree apply(long long k, const S& x) const { return set(k, op(get(k), x)); }

    /**
     * @brief [l, r)の区間を別の版の値にした新しい版を返す
     *
     * @param other 写す元の版 (自身と同じ木から作ったもの)
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @return PersistentSegTree 新しい版 (自身も other も変わらない)
     */
    PersistentSegTree copy(const PersistentSegTree& other, long long l, long long r) const {
        assert(pool == other.pool);
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return *this;
        return PersistentSegTree(pool, n, sz, copy_rec(root, other.root, 0, sz, l, r));
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
        return prod_rec(root, 0, sz, l, r);
    }
    /**
     * @brief すべての要素の総積を返す
     *
     * @return S 総積
     */
    S all_prod() const { return pool->val[root]; }

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
        long long r = max_right_rec(root, 0, sz, l, f, sum);
        return r < n ? r : n;
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
        return min_left_rec(root, 0, sz, r, f, sum);
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

template <typename M>
PersistentSegTree(long long, M) -> PersistentSegTree<M>;
template <typename M>
PersistentSegTree(std::vector<typename M::S>, M) -> PersistentSegTree<M>;
