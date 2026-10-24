#pragma once

/**
 * @file segtree.hpp
 * @brief セグメント木
 */

#include <cassert>
#include <ostream>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/data_structures/segtree.hpp

/**
 * @brief セグメント木
 *
 * 例:
 * SegTree<monoid::Max<int>> seg(n);
 * SegTree seg(v, Monoid([](int a, int b) { return a + b; }, 0));
 *
 * @tparam M モノイド (algebra/monoid.hpp)
 */
template <typename M>
class SegTree {
   public:
    using S = typename M::S;

   private:
    M m;
    S op(const S& a, const S& b) const { return m.op(a, b); }
    S e() const { return m.e(); }

    long long n, sz;
    int height;
    std::vector<S> data;
    void update(long long k) { data[k] = op(data[2 * k], data[2 * k + 1]); }

    class SegTreeReference {
        SegTree& segtree;
        long long k;

       public:
        SegTreeReference(SegTree& segtree, long long k) : segtree(segtree), k(k) {}
        SegTreeReference& operator=(const S& x) {
            segtree.set(k, x);
            return *this;
        }
        operator S() const { return segtree.get(k); }
    };

    void construct_data() {
        sz = 1;
        height = 0;
        while (sz < n) {
            sz <<= 1;
            height++;
        }
        data.assign(sz * 2, e());
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * @param n 要素数
     * @param m モノイド
     */
    explicit SegTree(long long n = 0, M m = M()) : m(std::move(m)), n(n) {
        construct_data();
    }
    /**
     * @brief コンストラクタ
     *
     * @param v 初期の要素
     * @param m モノイド
     */
    explicit SegTree(const std::vector<S>& v, M m = M()) : SegTree(v.size(), std::move(m)) {
        for (long long i = 0; i < n; i++) data[sz + i] = v[i];
        for (long long i = sz - 1; i > 0; i--) update(i);
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
        return data[sz + k];
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
     * @return SegTreeReference 要素への参照 代入されるとset()が呼ばれる
     */
    SegTreeReference operator[](long long k) { return SegTreeReference(*this, k); }

    /**
     * @brief 内容を出力する
     *
     * @tparam CharT 出力ストリームの文字型
     * @tparam Traits 出力ストリームの文字型特性
     * @param os 出力ストリーム
     * @param rhs セグメント木
     * @return std::basic_ostream<CharT, Traits>& 出力ストリーム
     */
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const SegTree& rhs) {
        for (long long i = 0; i < rhs.n; i++) {
            if (i != 0) os << CharT(' ');
            os << rhs[i];
        }
        return os;
    }

    /**
     * @brief 指定された要素の値をxに更新する
     *
     * @param k インデックス
     * @param x 新しい値
     */
    void set(long long k, const S& x) {
        assert(0 <= k && k < n);
        k += sz;
        data[k] = x;
        for (int i = 1; i <= height; i++) update(k >> i);
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
        S left_prod = e(), right_prod = e();
        l += sz;
        r += sz;
        while (l < r) {
            if (l & 1) left_prod = op(left_prod, data[l++]);
            if (r & 1) right_prod = op(data[--r], right_prod);
            l >>= 1;
            r >>= 1;
        }
        return op(left_prod, right_prod);
    }
    /**
     * @brief すべての要素の総積を返す
     *
     * @return S 総積
     */
    S all_prod() const { return data[1]; }

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
        l += sz;
        while (l % 2 == 0) l >>= 1;
        S sum = e();
        while (f(op(sum, data[l]))) {
            sum = op(sum, data[l]);
            l++;
            while (l % 2 == 0) l >>= 1;
            if (l == 1) return n;
        }
        while (l < sz) {
            if (!f(op(sum, data[l * 2])))
                l *= 2;
            else {
                sum = op(sum, data[l * 2]);
                l = l * 2 + 1;
            }
        }
        return l - sz;
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
        r += sz;
        S sum = e();
        do {
            r--;
            while (r > 1 && r % 2 == 1) r >>= 1;
            if (!f(op(data[r], sum))) {
                while (r < sz) {
                    r = r * 2 + 1;
                    if (f(op(data[r], sum))) {
                        sum = op(data[r], sum);
                        r--;
                    }
                }
                return r + 1 - sz;
            }
            sum = op(data[r], sum);
        } while ((r & -r) != r);
        return 0;
    }
};

template <typename M>
SegTree(long long, M) -> SegTree<M>;
template <typename M>
SegTree(std::vector<typename M::S>, M) -> SegTree<M>;

/**
 * @brief RangeMaxQuery
 *
 * @tparam S 型
 */
template <typename S>
using RMaxQ = SegTree<monoid::Max<S>>;
/**
 * @brief RangeMinQuery
 *
 * @tparam S 型
 */
template <typename S>
using RMinQ = SegTree<monoid::Min<S>>;
/**
 * @brief RangeSumQuery
 *
 * @tparam S 型
 */
template <typename S>
using RSumQ = SegTree<monoid::Sum<S>>;
/**
 * @brief RangeProdQuery
 *
 * @tparam S 型
 */
template <typename S>
using RProdQ = SegTree<monoid::Prod<S>>;
/**
 * @brief RangeXorQuery
 *
 * @tparam S 型
 */
template <typename S>
using RXorQ = SegTree<monoid::Xor<S>>;
/**
 * @brief RangeGcdQuery
 *
 * @tparam S 型
 */
template <typename S>
using RGcdQ = SegTree<monoid::Gcd<S>>;
