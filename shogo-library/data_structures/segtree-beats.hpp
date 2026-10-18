#pragma once

/**
 * @file segtree-beats.hpp
 * @brief Segment Tree Beats
 */

#include <algorithm>
#include <cassert>
#include <ostream>
#include <tuple>
#include <utility>
#include <vector>

#include "../algebra/acted-monoid.hpp"
#include "../algebra/beats-acted-monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/db9a2f114afe8539fdb02e5a44fc654c2297c5fa/src/data_structures/segtree-beats.hpp

/**
 * @brief Segment Tree Beats
 *
 * 作用素付きモノイド (algebra/acted-monoid.hpp) と同じ形の型を受け取るが、
 * mapping は std::pair<bool, S> (作用に失敗したか, 作用後の値) を返す
 * 失敗した場合は子に伝搬してから計算し直す
 *
 * @tparam A 作用素付きモノイド
 */
template <typename A>
class SegTreeBeats {
   public:
    using S = typename A::S;
    using F = typename A::F;

   private:
    A a;
    S op(const S& x, const S& y) const { return a.op(x, y); }
    S e() const { return a.e(); }
    std::pair<bool, S> mapping(const F& f, const S& x, long long l, long long r) const { return acted_monoid::apply_mapping(a, f, x, l, r); }
    F composition(const F& f, const F& g) const { return a.composition(f, g); }
    F id() const { return a.id(); }

    long long n, sz;
    int height;
    std::vector<S> data;
    std::vector<int> fail;
    std::vector<F> lazy;

    void update(long long k) {
        data[k] = op(data[2 * k], data[2 * k + 1]);
        fail[k] = false;
    }
    void apply_node(long long k, int h, const F& f) {
        // 末尾の要素数を 2 べきにそろえるための余りの部分は区間に含めない
        long long l = std::min((k << h) & (sz - 1), n);
        long long r = std::min(((k << h) & (sz - 1)) + (1LL << h), n);
        std::tie(fail[k], data[k]) = mapping(f, data[k], l, r);
        if (k < sz) {
            lazy[k] = composition(f, lazy[k]);
            if (fail[k]) {
                push(k, h);
                update(k);
            }
        }
    }
    void push(long long k, int h) {
        apply_node(2 * k, h - 1, lazy[k]);
        apply_node(2 * k + 1, h - 1, lazy[k]);
        lazy[k] = id();
    }

    class SegTreeBeatsReference {
        SegTreeBeats& segtree;
        long long k;

       public:
        SegTreeBeatsReference(SegTreeBeats& segtree, long long k) : segtree(segtree), k(k) {}
        SegTreeBeatsReference& operator=(const S& x) {
            segtree.set(k, x);
            return *this;
        }
        operator S() { return segtree.get(k); }
    };

    void construct_data() {
        sz = 1;
        height = 0;
        while (sz < n) {
            sz <<= 1;
            height++;
        }
        data.assign(sz * 2, e());
        fail.assign(sz * 2, false);
        lazy.assign(sz * 2, id());
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * @param n 要素数
     * @param a 作用素付きモノイド
     */
    explicit SegTreeBeats(long long n = 0, A a = A()) : a(std::move(a)), n(n) {
        construct_data();
    }
    /**
     * @brief コンストラクタ
     *
     * @param v 初期の要素
     * @param a 作用素付きモノイド
     */
    explicit SegTreeBeats(const std::vector<S>& v, A a = A()) : SegTreeBeats(v.size(), std::move(a)) {
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
    S get(long long k) {
        assert(0 <= k && k < n);
        k += sz;
        for (int h = height; h > 0; h--) {
            push(k >> h, h);
        }
        return data[k];
    }
    /**
     * @brief 指定された要素への参照を返す
     *
     * @param k
     * @return SegTreeReference 要素への参照 代入されるとset()が呼ばれる
     */
    SegTreeBeatsReference operator[](long long k) { return SegTreeBeatsReference(*this, k); }

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
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, SegTreeBeats& rhs) {
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
        for (int h = height; h > 0; h--) {
            push(k >> h, h);
        }
        data[k] = x;
        while (k >>= 1) update(k);
    }

    /**
     * @brief [l, r)の区間の総積を返す
     *
     * @param l 半開区間の開始
     * @param r 半開区間の終端
     * @return S 総積
     */
    S prod(long long l, long long r) {
        assert(0 <= l && l <= r && r <= n);
        l += sz;
        r += sz;
        for (int h = height; h > 0; h--) {
            if (((l >> h) << h) != l) push(l >> h, h);
            if (((r >> h) << h) != r) push(r >> h, h);
        }
        S left_prod = e(), right_prod = e();
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
     * @brief 指定された要素の値にxを作用させる
     *
     * @param k インデックス
     * @param x 作用素
     */
    void apply(long long k, const F& f) {
        assert(0 <= k && k < n);
        k += sz;
        for (int h = height; h > 0; h--) {
            push(k >> h, h);
        }
        data[k] = mapping(f, data[k], k - sz, k - sz + 1).second;
        while (k >>= 1) update(k);
    }
    /**
     * @brief [l, r)の区間の値にxを作用させる
     *
     * @param l 半開区間の開始
     * @param r 半開区間の終端
     * @param f 作用素
     */
    void apply(long long l, long long r, const F& f) {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return;
        l += sz;
        r += sz;
        for (int h = height; h > 0; h--) {
            if (((l >> h) << h) != l) push(l >> h, h);
            if (((r >> h) << h) != r) push(r >> h, h);
        }
        {
            long long l2 = l, r2 = r;
            int h = 0;
            while (l < r) {
                if (l & 1) apply_node(l++, h, f);
                if (r & 1) apply_node(--r, h, f);
                l >>= 1;
                r >>= 1;
                h++;
            }
            l = l2;
            r = r2;
        }
        for (int h = 1; h <= height; h++) {
            if (((l >> h) << h) != l) update(l >> h);
            if (((r >> h) << h) != r) update((r - 1) >> h);
        }
    }

    /**
     * @brief (r = l or g(prod([l, r))) = true) and (r = n or g(prod([l, r+1))) = false)となるrを返す
     * gが単調なら、g(prod([l, r))) = trueとなる最大のr
     *
     * @tparam G
     * @param l 半開区間の開始
     * @param g 判定関数 g(e) = true
     * @return long long
     */
    template <typename G>
    long long max_right(long long l, G g) {
        assert(g(e()));
        assert(0 <= l && l <= n);
        if (l == n) return n;
        l += sz;
        for (int h = height; h > 0; h--) {
            push(l >> h, h);
        }
        int h = 0;
        while (l % 2 == 0) {
            l >>= 1;
            h++;
        }
        S sum = e();
        while (g(op(sum, data[l]))) {
            sum = op(sum, data[l]);
            l++;
            while (l % 2 == 0) {
                l >>= 1;
                h++;
            }
            if (l == 1) return n;
        }
        while (l < sz) {
            push(l, h);
            if (!g(op(sum, data[l * 2]))) {
                l = l * 2;
            } else {
                sum = op(sum, data[l * 2]);
                l = l * 2 + 1;
            }
            h--;
        }
        return l - sz;
    }
    /**
     * @brief (l = 0 or g(prod([l, r))) = true) and (l = r or g(prod([l-1, r))) = false)となるlを返す
     * gが単調なら、g(prod([l, r))) = trueとなる最小のl
     *
     * @tparam G
     * @param r 半開区間の終端
     * @param g 判定関数 g(e) = true
     * @return long long
     */
    template <typename G>
    long long min_left(long long r, G g) {
        assert(g(e()));
        assert(0 <= r && r <= n);
        if (r == 0) return 0;
        r += sz;
        for (int h = height; h > 0; h--) {
            push((r - 1) >> h, h);
        }
        int h = 0;
        S sum = e();
        do {
            r--;
            while (r > 1 && r % 2 == 1) {
                r >>= 1;
                h++;
            }
            if (!g(op(data[r], sum))) {
                while (r < sz) {
                    push(r, h);
                    r = r * 2 + 1;
                    h--;
                    if (g(op(data[r], sum))) {
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

template <typename A>
SegTreeBeats(long long, A) -> SegTreeBeats<A>;
template <typename A>
SegTreeBeats(std::vector<typename A::S>, A) -> SegTreeBeats<A>;

/**
 * @brief 区間 chmin、区間和・最大値
 *
 * @tparam S 型
 */
template <typename S>
using RChminSumQ = SegTreeBeats<beats_acted_monoid::ChminSum<S>>;
/**
 * @brief 区間 chmax、区間和・最小値
 *
 * @tparam S 型
 */
template <typename S>
using RChmaxSumQ = SegTreeBeats<beats_acted_monoid::ChmaxSum<S>>;
/**
 * @brief 区間 chmin・chmax・加算・代入、区間和・最大値・最小値
 *
 * @tparam S 型
 */
template <typename S>
using RChminChmaxAddSumQ = SegTreeBeats<beats_acted_monoid::ChminChmaxAddSum<S>>;
