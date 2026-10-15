#pragma once

/**
 * @file imos.hpp
 * @brief imos法で区間作用、1点取得
 */

#include <cassert>
#include <utility>
#include <vector>

#include "../algebra/acted-monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/data_structures/imos.hpp

/**
 * @brief imos法で区間作用、1点取得する。
 *
 * 例: Imos<acted_monoid::AddSum<long long>> imos(n);
 *
 * @tparam A 作用素付きモノイド (algebra/acted-monoid.hpp)
 *           作用素はアーベル群(可換群)をなし、inv を持つ必要がある 値のモノイドの演算は使わない
 */
template <typename A>
class Imos {
   public:
    using S = typename A::S;
    using F = typename A::F;

   private:
    A a;
    long long _n, t;
    std::vector<S> data;
    std::vector<F> lazy;

    S mapping(const F& f, const S& x, long long k) const { return acted_monoid::apply_mapping(a, f, x, k, k + 1); }
    F composition(const F& f, const F& g) const { return a.composition(f, g); }
    F id() const { return a.id(); }
    F inv(const F& f) const { return a.inv(f); }

   public:
    /**
     * @param n 要素数 各要素は S() で初期化する
     * @param a 作用素付きモノイド
     */
    explicit Imos(long long n = 0, A a = A()) : Imos(n, S(), std::move(a)) {}
    /**
     * @param n 要素数
     * @param value 各要素の初期値
     * @param a 作用素付きモノイド
     */
    Imos(long long n, const S& value, A a = A()) : a(std::move(a)), _n(n), t(n), data(n, value), lazy(n, this->a.id()) {}
    /**
     * @param value 各要素の初期値
     * @param a 作用素付きモノイド
     */
    explicit Imos(const std::vector<S>& value, A a = A()) : a(std::move(a)), _n(value.size()), t(value.size()), data(value), lazy(value.size(), this->a.id()) {}

    /**
     * @brief 1点作用
     * @param k インデックス
     * @param f 作用素
     */
    void apply(long long k, const F& f) {
        assert(0 <= k && k < _n);
        data[k] = mapping(f, data[k], k);
    }
    /**
     * @brief [l, r)の区間の値にfを作用させる
     * @param l 半開区間の開始
     * @param r 半開区間の終端 0<=l<=r<=n
     * @param f 作用素
     */
    void apply(long long l, long long r, const F& f) {
        assert(0 <= l && l <= r && r <= _n);
        if (l < _n) lazy[l] = composition(lazy[l], f);
        if (r < _n) lazy[r] = composition(lazy[r], inv(f));
        if (t > l) t = l;
    }
    /**
     * @brief 指定された要素の値を返す
     * @param k インデックス
     * @return const S& 値
     */
    const S& get(long long k) {
        assert(0 <= k && k < _n);
        while (t <= k) {
            data[t] = mapping(lazy[t], data[t], t);
            if (t + 1 < _n) {
                lazy[t + 1] = composition(lazy[t], lazy[t + 1]);
            }
            lazy[t] = id();
            t++;
        }
        return data[k];
    }
};

template <typename A>
Imos(long long, A) -> Imos<A>;
template <typename A>
Imos(long long, typename A::S, A) -> Imos<A>;
template <typename A>
Imos(std::vector<typename A::S>, A) -> Imos<A>;
