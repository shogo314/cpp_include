#pragma once

/**
 * @file dual-segtree.hpp
 * @brief 双対セグメント木 (区間作用、1点取得)
 */

#include <cassert>
#include <utility>
#include <vector>

#include "../algebra/acted-monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/data_structures/dual-segtree.hpp

/**
 * @brief 双対セグメント木 (区間作用、1点取得)
 *
 * 区間への作用素の適用と1点の取得を O(log n) で行う。作用素は可換でなくてもよい
 * 遅延伝搬セグメント木と違い、値のモノイドの演算 (op、e) は使わない
 * 例:
 * DualSegTree<acted_monoid::AffineSum<mint>> seg(v);
 * seg.apply(l, r, Affine<mint>{b, c});
 * mint x = seg.get(i);
 *
 * @tparam A 作用素付きモノイド (algebra/acted-monoid.hpp)
 */
template <typename A>
class DualSegTree {
   public:
    using S = typename A::S;
    using F = typename A::F;

   private:
    A a;
    long long n, sz;
    int height;
    std::vector<S> data;  // 葉の値 (作用素をまだ適用していないもの)
    std::vector<F> lazy;  // 各ノードの区間に、まだ下へ伝えていない作用素

    S mapping(const F& f, const S& x, long long k) const { return acted_monoid::apply_mapping(a, f, x, k, k + 1); }
    F composition(const F& f, const F& g) const { return a.composition(f, g); }
    F id() const { return a.id(); }

    void apply_node(long long k, const F& f) { lazy[k] = composition(f, lazy[k]); }
    void push(long long k) {
        apply_node(2 * k, lazy[k]);
        apply_node(2 * k + 1, lazy[k]);
        lazy[k] = id();
    }
    // 葉 k の祖先の作用素をすべて下へ伝える
    void push_ancestors(long long k) {
        for (int h = height; h > 0; h--) push(k >> h);
    }

    void construct() {
        sz = 1;
        height = 0;
        while (sz < n) {
            sz <<= 1;
            height++;
        }
        lazy.assign(2 * sz, id());
    }

   public:
    /**
     * @param n 要素数 各要素は S() で初期化する
     * @param a 作用素付きモノイド
     */
    explicit DualSegTree(long long n = 0, A a = A()) : DualSegTree(n, S(), std::move(a)) {}
    /**
     * @param n 要素数
     * @param value 各要素の初期値
     * @param a 作用素付きモノイド
     */
    DualSegTree(long long n, const S& value, A a = A()) : a(std::move(a)), n(n), data(n, value) { construct(); }
    /**
     * @param v 初期の要素
     * @param a 作用素付きモノイド
     */
    explicit DualSegTree(const std::vector<S>& v, A a = A()) : a(std::move(a)), n(v.size()), data(v) { construct(); }

    /**
     * @brief 要素数を返す
     */
    long long size() const { return n; }

    /**
     * @brief 指定された要素の値を返す O(log n)
     */
    S get(long long k) const {
        assert(0 <= k && k < n);
        // 根に近いノードほど後に作用した作用素なので、葉から順に合成する
        F f = id();
        for (long long i = k + sz; i > 0; i >>= 1) f = composition(lazy[i], f);
        return mapping(f, data[k], k);
    }
    /**
     * @brief すべての要素の値を返す O(n)
     */
    std::vector<S> to_vector() {
        for (long long k = 1; k < sz; k++) push(k);
        std::vector<S> res(n);
        for (long long k = 0; k < n; k++) res[k] = mapping(lazy[k + sz], data[k], k);
        return res;
    }
    /**
     * @brief 指定された要素の値を x にする O(log n)
     */
    void set(long long k, const S& x) {
        assert(0 <= k && k < n);
        push_ancestors(k + sz);
        data[k] = x;
        lazy[k + sz] = id();
    }

    /**
     * @brief 1点作用 O(log n)
     */
    void apply(long long k, const F& f) {
        assert(0 <= k && k < n);
        push_ancestors(k + sz);
        apply_node(k + sz, f);
    }
    /**
     * @brief [l, r) の区間の値に f を作用させる O(log n)
     */
    void apply(long long l, long long r, const F& f) {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return;
        l += sz;
        r += sz;
        // 作用する区間の境界の祖先に残っている作用素を先に下へ伝える (作用素が可換でなくても順序を保つため)
        for (int h = height; h > 0; h--) {
            if (((l >> h) << h) != l) push(l >> h);
            if (((r >> h) << h) != r) push((r - 1) >> h);
        }
        for (; l < r; l >>= 1, r >>= 1) {
            if (l & 1) apply_node(l++, f);
            if (r & 1) apply_node(--r, f);
        }
    }
};

template <typename A>
DualSegTree(long long, A) -> DualSegTree<A>;
template <typename A>
DualSegTree(long long, typename A::S, A) -> DualSegTree<A>;
template <typename A>
DualSegTree(std::vector<typename A::S>, A) -> DualSegTree<A>;

// 別名 双対セグメント木は op, e を使わないので、取得の部分を名前に含めない
// mapping が区間の長さを使わない Min 系を実体にする (値の型に S(sz) や掛け算を求めない)

/**
 * @brief 区間加算、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RAddQ = DualSegTree<acted_monoid::AddMin<S>>;
/**
 * @brief 区間代入、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RUpdateQ = DualSegTree<acted_monoid::UpdateMin<S>>;
/**
 * @brief 区間 chmin、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RChminQ = DualSegTree<acted_monoid::ChminMin<S>>;
/**
 * @brief 区間 chmax、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RChmaxQ = DualSegTree<acted_monoid::ChmaxMax<S>>;
/**
 * @brief 区間アフィン変換 x <- a * x + b (Affine)、1点取得
 *
 * @tparam S 型
 */
template <typename S>
using RAffineQ = DualSegTree<acted_monoid::AffineSum<S>>;
