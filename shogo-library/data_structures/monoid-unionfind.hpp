#pragma once

/**
 * @file monoid-unionfind.hpp
 * @brief 可換モノイドを乗せるUnionFind
 */

#include <cassert>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <vector>

#include "../algebra/monoid.hpp"
#include "../utility/traits.hpp"
#include "unionfind.hpp"
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/data_structures/monoid-unionfind.hpp

/**
 * @brief 可換モノイドを乗せるUnionFind
 *
 * @tparam M 可換モノイド (algebra/monoid.hpp)
 */
template <typename M>
class MonoidUnionFind : private UnionFind {
   public:
    using S = typename M::S;

   private:
    M m;
    std::vector<S> val;
    using UnionFind::_n;
    using UnionFind::parent_or_size;

   public:
    /**
     * @param n 要素数 各要素の値は単位元で初期化する
     * @param m 可換モノイド
     */
    explicit MonoidUnionFind(long long n = 0, M m = M()) : UnionFind(n), m(std::move(m)), val(n, this->m.e()) {}
    /**
     * @param v 各要素の初期値
     * @param m 可換モノイド
     */
    explicit MonoidUnionFind(std::vector<S> v, M m = M()) : UnionFind(v.size()), m(std::move(m)), val(std::move(v)) {}

    /**
     * @brief 各要素を S(x) で構築して初期化する
     */
#if __cplusplus >= 202002L
    template <class R = std::initializer_list<S>>
        requires internal_traits::sized_constructible_range<R, S>
#else
    template <class R = std::initializer_list<S>, ENABLE_IF_T(internal_traits::is_sized_constructible_range<R, S>::value)>
#endif
    explicit MonoidUnionFind(R&& v, M m = M())
        : UnionFind(internal_traits::range_size(v)), m(std::move(m)) {
        val.reserve(internal_traits::range_size(v));
        for (auto&& x : v) val.emplace_back(std::forward<decltype(x)>(x));
    }

    using MergeResult = UnionFind::MergeResult;
    using UnionFind::groups;
    using UnionFind::leader;
    using UnionFind::same;
    using UnionFind::size;
    /**
     * @brief a と b の連結成分を統合し、値の積を取る
     * @return 統合後の代表元と、新しく統合したか
     */
    MergeResult merge(long long a, long long b) {
        assert(0 <= a && a < _n);
        assert(0 <= b && b < _n);
        long long x = leader(a), y = leader(b);
        if (x == y) return {x, false};
        if (-parent_or_size[x] < -parent_or_size[y]) std::swap(x, y);
        parent_or_size[x] += parent_or_size[y];
        parent_or_size[y] = static_cast<int>(x);
        val[x] = m.op(val[x], val[y]);
        return {x, true};
    }
    /**
     * @brief 要素aの属する連結成分の値の総積
     */
    const S& prod(long long a) {
        return val[leader(a)];
    }
};

template <typename M>
MonoidUnionFind(long long, M) -> MonoidUnionFind<M>;
template <class R, typename M>
MonoidUnionFind(R&&, M) -> MonoidUnionFind<M>;
template <typename M>
MonoidUnionFind(std::initializer_list<typename M::S>, M) -> MonoidUnionFind<M>;
