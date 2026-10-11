#pragma once

/**
 * @file monoid.hpp
 * @brief モノイドの定義
 *
 * モノイドは次のメンバを持つ型として表す
 * - using S: 要素の型
 * - S op(const S& a, const S& b): 積 (結合的)
 * - S e(): 単位元
 * op, e は static でも非 static でもよい (データ構造はモノイドのオブジェクトを保持して呼び出す)
 */

#include <algorithm>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

namespace monoid {

/**
 * @brief 和
 */
template <typename T>
struct Sum {
    using S = T;
    static constexpr S op(const S& a, const S& b) { return a + b; }
    static constexpr S e() { return S(0); }
};

/**
 * @brief 積
 */
template <typename T>
struct Prod {
    using S = T;
    static constexpr S op(const S& a, const S& b) { return a * b; }
    static constexpr S e() { return S(1); }
};

/**
 * @brief 最大値 単位元は型の最小値
 */
template <typename T>
struct Max {
    using S = T;
    static constexpr S op(const S& a, const S& b) { return std::max(a, b); }
    static constexpr S e() { return std::numeric_limits<S>::lowest(); }
};

/**
 * @brief 最小値 単位元は型の最大値
 */
template <typename T>
struct Min {
    using S = T;
    static constexpr S op(const S& a, const S& b) { return std::min(a, b); }
    static constexpr S e() { return std::numeric_limits<S>::max(); }
};

/**
 * @brief 最小値と最大値の組 {min, max}
 */
template <typename T>
struct MinMax {
    using S = std::pair<T, T>;
    static constexpr S op(const S& a, const S& b) { return {std::min(a.first, b.first), std::max(a.second, b.second)}; }
    static constexpr S e() { return {std::numeric_limits<T>::max(), std::numeric_limits<T>::lowest()}; }
};

/**
 * @brief 排他的論理和
 */
template <typename T>
struct Xor {
    using S = T;
    static constexpr S op(const S& a, const S& b) { return a ^ b; }
    static constexpr S e() { return S(0); }
};

/**
 * @brief 論理積
 */
template <typename T>
struct And {
    using S = T;
    static constexpr S op(const S& a, const S& b) { return a & b; }
    static constexpr S e() { return ~S(0); }
};

/**
 * @brief 論理和
 */
template <typename T>
struct Or {
    using S = T;
    static constexpr S op(const S& a, const S& b) { return a | b; }
    static constexpr S e() { return S(0); }
};

/**
 * @brief 最大公約数 単位元は0
 */
template <typename T>
struct Gcd {
    static_assert(std::is_integral_v<T>);
    using S = T;
    static constexpr S op(const S& a, const S& b) { return std::gcd(a, b); }
    static constexpr S e() { return S(0); }
};

/**
 * @brief 最小公倍数 単位元は1
 */
template <typename T>
struct Lcm {
    static_assert(std::is_integral_v<T>);
    using S = T;
    static constexpr S op(const S& a, const S& b) { return std::lcm(a, b); }
    static constexpr S e() { return S(1); }
};

/**
 * @brief 1次関数 f(x) = first * x + second の合成
 * op(f, g) = g∘f (左から順に適用する)
 * 区間 [l, r) の積は f_{r-1}(...f_l(x)) を表す
 */
template <typename T>
struct Composite {
    using S = std::pair<T, T>;
    static constexpr S op(const S& f, const S& g) { return {g.first * f.first, g.first * f.second + g.second}; }
    static constexpr S e() { return {T(1), T(0)}; }
};

/**
 * @brief 括弧列 対応の取れていない ')' と '(' の個数
 */
struct Parenthesis {
    struct S {
        long long close; // 対応の取れていない ')' の個数
        long long open;  // 対応の取れていない '(' の個数
        constexpr S(char c = 0) : close(c == ')'), open(c == '(') {}
        constexpr S(long long close, long long open) : close(close), open(open) {}
        /**
         * @brief 正しい括弧列か
         */
        constexpr bool is_valid() const { return close == 0 && open == 0; }
    };
    static constexpr S op(const S& a, const S& b) {
        if(a.open >= b.close) return S(a.close, a.open - b.close + b.open);
        return S(a.close + b.close - a.open, b.open);
    }
    static constexpr S e() { return S(); }
};

/**
 * @brief 最小値とその位置 同じ値なら左側を優先
 *
 * @tparam T 値の型
 * @tparam U 位置などの付随する値の型
 */
template <typename T, typename U = int>
struct MinWith {
    using S = std::pair<T, U>;
    static constexpr S op(const S& a, const S& b) { return b.first < a.first ? b : a; }
    static constexpr S e() { return {std::numeric_limits<T>::max(), U{}}; }
};

/**
 * @brief 最大値とその位置 同じ値なら左側を優先
 *
 * @tparam T 値の型
 * @tparam U 位置などの付随する値の型
 */
template <typename T, typename U = int>
struct MaxWith {
    using S = std::pair<T, U>;
    static constexpr S op(const S& a, const S& b) { return a.first < b.first ? b : a; }
    static constexpr S e() { return {std::numeric_limits<T>::lowest(), U{}}; }
};

/**
 * @brief 最小値とその個数
 *
 * @tparam T 値の型
 * @tparam U 個数の型
 */
template <typename T, typename U = long long>
struct MinCount {
    using S = std::pair<T, U>;
    static constexpr S op(const S& a, const S& b) {
        if(a.first < b.first) return a;
        if(b.first < a.first) return b;
        return {a.first, a.second + b.second};
    }
    static constexpr S e() { return {std::numeric_limits<T>::max(), U(0)}; }
};

/**
 * @brief 最大値とその個数
 *
 * @tparam T 値の型
 * @tparam U 個数の型
 */
template <typename T, typename U = long long>
struct MaxCount {
    using S = std::pair<T, U>;
    static constexpr S op(const S& a, const S& b) {
        if(a.first < b.first) return b;
        if(b.first < a.first) return a;
        return {a.first, a.second + b.second};
    }
    static constexpr S e() { return {std::numeric_limits<T>::lowest(), U(0)}; }
};

} // namespace monoid

/**
 * @brief 関数オブジェクトと単位元から実行時に作るモノイド
 * ラムダ式を使える
 *
 * 例: SegTree seg(v, Monoid([](int a, int b) { return std::max(a, b); }, -1));
 *
 * @tparam T 要素の型
 * @tparam Op 積の関数オブジェクトの型
 */
template <typename T, typename Op>
struct Monoid {
    using S = T;
    Op op_object;
    S identity;
    Monoid() = default;
    constexpr Monoid(Op op, const S& e) : op_object(std::move(op)), identity(e) {}
    constexpr S op(const S& a, const S& b) const { return op_object(a, b); }
    constexpr S e() const { return identity; }
};
template <typename Op, typename T>
Monoid(Op, T) -> Monoid<T, Op>;
