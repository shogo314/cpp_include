#pragma once

/**
 * @file group.hpp
 * @brief 群の定義
 *
 * 群はモノイド (algebra/monoid.hpp) のメンバに加えて次のメンバを持つ型として表す
 * - S inv(const S& a): 逆元
 */

#include <array>
#include <cstddef>
#include <utility>

#include "monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/a2fefc7ec36d798ade24522d799381341eac20dd/src/algebra/group.hpp

namespace group {

/**
 * @brief 加法群
 */
template <typename T>
struct Sum : monoid::Sum<T> {
    using S = T;
    static constexpr S inv(const S& a) { return -a; }
};

/**
 * @brief 乗法群 (0を含まない体の元など)
 */
template <typename T>
struct Prod : monoid::Prod<T> {
    using S = T;
    static constexpr S inv(const S& a) { return S(1) / a; }
};

/**
 * @brief 排他的論理和
 */
template <typename T>
struct Xor : monoid::Xor<T> {
    using S = T;
    static constexpr S inv(const S& a) { return a; }
};

/**
 * @brief {0, 1, ..., N-1} の置換 (全単射) の合成
 *
 * monoid::Mapping<N> に逆置換を加えたもの。要素は置換であること
 * op(f, g) = f∘g (g を先に適用する)
 *
 * @tparam N 置換する要素の個数
 */
template <std::size_t N>
struct Permutation : monoid::Mapping<N> {
    using S = std::array<int, N>;
    static constexpr S inv(const S& f) {
        S h{};
        for (std::size_t x = 0; x < N; x++) h[f[x]] = static_cast<int>(x);
        return h;
    }
};

}  // namespace group

/**
 * @brief 関数オブジェクトと単位元から実行時に作る群
 * ラムダ式を使える
 *
 * @tparam T 要素の型
 * @tparam Op 積の関数オブジェクトの型
 * @tparam Inv 逆元の関数オブジェクトの型
 */
template <typename T, typename Op, typename Inv>
struct Group {
    using S = T;
    Op op_object;
    S identity;
    Inv inv_object;
    Group() = default;
    constexpr Group(Op op, const S& e, Inv inv) : op_object(std::move(op)), identity(e), inv_object(std::move(inv)) {}
    constexpr S op(const S& a, const S& b) const { return op_object(a, b); }
    constexpr S e() const { return identity; }
    constexpr S inv(const S& a) const { return inv_object(a); }
};
template <typename Op, typename T, typename Inv>
Group(Op, T, Inv) -> Group<T, Op, Inv>;
