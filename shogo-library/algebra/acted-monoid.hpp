#pragma once

/**
 * @file acted-monoid.hpp
 * @brief 作用素付きモノイドの定義
 *
 * 作用素付きモノイドはモノイド (algebra/monoid.hpp) のメンバに加えて次のメンバを持つ型として表す
 * - using F: 作用素の型
 * - S mapping(const F& f, const S& x): 作用素の適用 f(x)
 *   区間の情報が必要な場合は mapping(f, x, sz) (sz は区間の長さ) や mapping(f, x, l, r) (区間 [l, r)) でもよい
 *   sz, l, r は long long で渡す (動的セグメント木では int に収まらないことがある)
 * - F composition(const F& f, const F& g): 作用素の合成 f∘g (gを適用した後にfを適用する)
 * - F id(): 恒等写像
 * 作用素が群をなす場合 (imos.hpp で使う) は
 * - F inv(const F& f): 逆作用素
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

#include "monoid.hpp"
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/algebra/acted-monoid.hpp

namespace internal_acted_monoid {
template <typename A, typename = void>
struct has_mapping_lr : std::false_type {};
template <typename A>
struct has_mapping_lr<A, std::void_t<decltype(std::declval<const A&>().mapping(std::declval<const typename A::F&>(), std::declval<const typename A::S&>(), 0, 0))>> : std::true_type {};
template <typename A, typename = void>
struct has_mapping_sz : std::false_type {};
template <typename A>
struct has_mapping_sz<A, std::void_t<decltype(std::declval<const A&>().mapping(std::declval<const typename A::F&>(), std::declval<const typename A::S&>(), 0))>> : std::true_type {};
}  // namespace internal_acted_monoid

namespace acted_monoid {

/**
 * @brief 区間 [l, r) を表す要素に作用素を適用する
 * mapping の引数の形の違いを吸収する
 */
template <typename A>
constexpr auto apply_mapping(const A& a, const typename A::F& f, const typename A::S& x, long long l, long long r) {
    if constexpr (internal_acted_monoid::has_mapping_lr<A>::value) {
        return a.mapping(f, x, l, r);
    } else if constexpr (internal_acted_monoid::has_mapping_sz<A>::value) {
        return a.mapping(f, x, r - l);
    } else {
        return a.mapping(f, x);
    }
}

/**
 * @brief 区間加算、区間和
 */
template <typename T>
struct AddSum : monoid::Sum<T> {
    using S = T;
    using F = T;
    static constexpr S mapping(const F& f, const S& x, long long sz) { return x + f * S(sz); }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
    static constexpr F inv(const F& f) { return -f; }
};

/**
 * @brief 区間乗算、区間和
 */
template <typename T>
struct MulSum : monoid::Sum<T> {
    using S = T;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return f * x; }
    static constexpr F composition(const F& f, const F& g) { return f * g; }
    static constexpr F id() { return F(1); }
    static constexpr F inv(const F& f) { return F(1) / f; }
};

/**
 * @brief 区間加算、区間最小値
 */
template <typename T>
struct AddMin : monoid::Min<T> {
    using S = T;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return x + f; }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
};

/**
 * @brief 区間加算、区間最大値
 */
template <typename T>
struct AddMax : monoid::Max<T> {
    using S = T;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return x + f; }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
};

/**
 * @brief 区間代入、区間和
 */
template <typename T>
struct UpdateSum : monoid::Sum<T> {
    using S = T;
    using F = std::optional<T>;
    static constexpr S mapping(const F& f, const S& x, long long sz) { return f ? *f * S(sz) : x; }
    static constexpr F composition(const F& f, const F& g) { return f ? f : g; }
    static constexpr F id() { return std::nullopt; }
};

/**
 * @brief 区間代入、区間最小値
 */
template <typename T>
struct UpdateMin : monoid::Min<T> {
    using S = T;
    using F = std::optional<T>;
    static constexpr S mapping(const F& f, const S& x) { return f ? *f : x; }
    static constexpr F composition(const F& f, const F& g) { return f ? f : g; }
    static constexpr F id() { return std::nullopt; }
};

/**
 * @brief 区間代入、区間最大値
 */
template <typename T>
struct UpdateMax : monoid::Max<T> {
    using S = T;
    using F = std::optional<T>;
    static constexpr S mapping(const F& f, const S& x) { return f ? *f : x; }
    static constexpr F composition(const F& f, const F& g) { return f ? f : g; }
    static constexpr F id() { return std::nullopt; }
};

/**
 * @brief 区間chmin、区間最小値
 */
template <typename T>
struct ChminMin : monoid::Min<T> {
    using S = T;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return std::min(f, x); }
    static constexpr F composition(const F& f, const F& g) { return std::min(f, g); }
    static constexpr F id() { return std::numeric_limits<F>::max(); }
};

/**
 * @brief 区間chmax、区間最大値
 */
template <typename T>
struct ChmaxMax : monoid::Max<T> {
    using S = T;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return std::max(f, x); }
    static constexpr F composition(const F& f, const F& g) { return std::max(f, g); }
    static constexpr F id() { return std::numeric_limits<F>::lowest(); }
};

/**
 * @brief 区間 clamp x <- min(max(x, lb), ub) (Clamp)、区間最小値
 * clamp は単調非減少なので、最小値に適用すればよい
 */
template <typename T>
struct ClampMin : monoid::Min<T> {
    using S = T;
    using F = Clamp<T>;
    static constexpr S mapping(const F& f, const S& x) { return f(x); }
    static constexpr F composition(const F& f, const F& g) { return monoid::ClampComposite<T>::op(f, g); }
    static constexpr F id() { return {}; }
};

/**
 * @brief 区間 clamp x <- min(max(x, lb), ub) (Clamp)、区間最大値
 */
template <typename T>
struct ClampMax : monoid::Max<T> {
    using S = T;
    using F = Clamp<T>;
    static constexpr S mapping(const F& f, const S& x) { return f(x); }
    static constexpr F composition(const F& f, const F& g) { return monoid::ClampComposite<T>::op(f, g); }
    static constexpr F id() { return {}; }
};

/**
 * @brief 区間 clamp x <- min(max(x, lb), ub) (Clamp)、区間最小値と最大値の組 {min, max} (monoid::MinMax)
 */
template <typename T>
struct ClampMinMax : monoid::MinMax<T> {
    using S = std::pair<T, T>;
    using F = Clamp<T>;
    static constexpr S mapping(const F& f, const S& x) { return {f(x.first), f(x.second)}; }
    static constexpr F composition(const F& f, const F& g) { return monoid::ClampComposite<T>::op(f, g); }
    static constexpr F id() { return {}; }
};

/**
 * @brief 区間 加算と clamp x <- min(max(x + add, lb), ub) (ClampAdd)、区間最小値
 * 区間加算・chmin・chmax・代入をまとめて扱える。単調非減少なので、最小値に適用すればよい
 */
template <typename T>
struct ClampAddMin : monoid::Min<T> {
    using S = T;
    using F = ClampAdd<T>;
    static constexpr S mapping(const F& f, const S& x) { return f(x); }
    static constexpr F composition(const F& f, const F& g) { return monoid::ClampAddComposite<T>::op(f, g); }
    static constexpr F id() { return {}; }
};

/**
 * @brief 区間 加算と clamp x <- min(max(x + add, lb), ub) (ClampAdd)、区間最大値
 */
template <typename T>
struct ClampAddMax : monoid::Max<T> {
    using S = T;
    using F = ClampAdd<T>;
    static constexpr S mapping(const F& f, const S& x) { return f(x); }
    static constexpr F composition(const F& f, const F& g) { return monoid::ClampAddComposite<T>::op(f, g); }
    static constexpr F id() { return {}; }
};

/**
 * @brief 区間 加算と clamp x <- min(max(x + add, lb), ub) (ClampAdd)、区間最小値と最大値の組 {min, max} (monoid::MinMax)
 */
template <typename T>
struct ClampAddMinMax : monoid::MinMax<T> {
    using S = std::pair<T, T>;
    using F = ClampAdd<T>;
    static constexpr S mapping(const F& f, const S& x) { return {f(x.first), f(x.second)}; }
    static constexpr F composition(const F& f, const F& g) { return monoid::ClampAddComposite<T>::op(f, g); }
    static constexpr F id() { return {}; }
};

/**
 * @brief 区間アフィン変換 x <- a * x + b (Affine)、区間和
 */
template <typename T>
struct AffineSum : monoid::Sum<T> {
    using S = T;
    using F = Affine<T>;
    static constexpr S mapping(const F& f, const S& x, long long sz) { return f.a * x + f.b * S(sz); }
    static constexpr F composition(const F& f, const F& g) { return {f.a * g.a, f.a * g.b + f.b}; }
    static constexpr F id() { return {T(1), T(0)}; }
};

/**
 * @brief 区間代入、1次関数の合成 (monoid::Composite)
 */
template <typename T>
struct UpdateComposite : monoid::Composite<T> {
    using S = Affine<T>;
    using F = std::optional<S>;
    static constexpr S mapping(const F& f, const S& x, long long sz) {
        if (!f) return x;
        // f を sz 回合成する
        S ret = monoid::Composite<T>::e(), p = *f;
        for (; sz > 0; sz >>= 1) {
            if (sz & 1) ret = monoid::Composite<T>::op(ret, p);
            p = monoid::Composite<T>::op(p, p);
        }
        return ret;
    }
    static constexpr F composition(const F& f, const F& g) { return f ? f : g; }
    static constexpr F id() { return std::nullopt; }
};

/**
 * @brief 区間加算、区間最小値とその位置 (monoid::MinWith)
 */
template <typename T, typename U = long long>
struct AddMinWith : monoid::MinWith<T, U> {
    using S = std::pair<T, U>;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return {x.first + f, x.second}; }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
};

/**
 * @brief 区間加算、区間最大値とその位置 (monoid::MaxWith)
 */
template <typename T, typename U = long long>
struct AddMaxWith : monoid::MaxWith<T, U> {
    using S = std::pair<T, U>;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return {x.first + f, x.second}; }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
};

/**
 * @brief 区間加算、区間最小値とその個数 (monoid::MinCount)
 */
template <typename T, typename U = long long>
struct AddMinCount : monoid::MinCount<T, U> {
    using S = std::pair<T, U>;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return {x.first + f, x.second}; }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
};

/**
 * @brief 区間加算、区間最大値とその個数 (monoid::MaxCount)
 */
template <typename T, typename U = long long>
struct AddMaxCount : monoid::MaxCount<T, U> {
    using S = std::pair<T, U>;
    using F = T;
    static constexpr S mapping(const F& f, const S& x) { return {x.first + f, x.second}; }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
};

/**
 * @brief 区間加算、区間の0乗和からK乗和
 *
 * S(x) で値 x の1要素を表す要素を作る。S::operator[](i) で i 乗和を取得する
 */
template <typename T, int K>
struct AddPowerSum {
    struct S {
        std::array<T, K + 1> val;
        constexpr S() : val{} {}
        constexpr S(const T& x) : val{} {
            val[0] = T(1);
            for (int i = 1; i <= K; i++) val[i] = val[i - 1] * x;
        }
        constexpr const T& operator[](long long i) const { return val[i]; }
    };
    using F = T;
    static constexpr S op(const S& a, const S& b) {
        S ret;
        for (int i = 0; i <= K; i++) ret.val[i] = a.val[i] + b.val[i];
        return ret;
    }
    static constexpr S e() { return S(); }
    static S mapping(const F& f, const S& x) {
        // sum (x+f)^i = sum_j binom(i, j) f^j x^(i-j)
        static const auto binom = [] {
            std::array<std::array<T, K + 1>, K + 1> c{};
            for (int i = 0; i <= K; i++) {
                c[i][0] = c[i][i] = T(1);
                for (int j = 1; j < i; j++) c[i][j] = c[i - 1][j - 1] + c[i - 1][j];
            }
            return c;
        }();
        std::array<T, K + 1> pw{};
        pw[0] = T(1);
        for (int i = 1; i <= K; i++) pw[i] = pw[i - 1] * f;
        S ret;
        for (int i = 0; i <= K; i++) {
            for (int j = 0; j <= i; j++) ret.val[i] += binom[i][j] * pw[j] * x.val[i - j];
        }
        return ret;
    }
    static constexpr F composition(const F& f, const F& g) { return f + g; }
    static constexpr F id() { return F(0); }
};

/**
 * @brief 成分の並べ替え (monoid::Mapping)、モノイド M の N 個の直積
 *
 * 要素は M の要素を N 個並べた std::array で、積は成分ごとに M::op をとる
 * 作用素 f は monoid::Mapping<N> の要素で、mapping(f, x)[i] = x[f[i]] (成分 i に x の成分 f[i] を持ってくる)
 * f は全単射でなくてよい。例えば N = 3 で f = {1, 1, 2} は成分 0 を成分 1 で上書きする
 * 積が成分ごとなので、M によらず作用は積と可換になる
 *
 * @tparam M static な op, e を持つモノイド
 * @tparam N 成分の個数
 */
template <typename M, std::size_t N>
struct Rearrange {
    using S = std::array<typename M::S, N>;
    using F = typename monoid::Mapping<N>::S;
    static constexpr S op(const S& a, const S& b) {
        S res{};
        for (std::size_t i = 0; i < N; i++) res[i] = M::op(a[i], b[i]);
        return res;
    }
    static constexpr S e() {
        S res{};
        for (std::size_t i = 0; i < N; i++) res[i] = M::e();
        return res;
    }
    static constexpr S mapping(const F& f, const S& x) {
        S res{};
        for (std::size_t i = 0; i < N; i++) res[i] = x[f[i]];
        return res;
    }
    // mapping(f, mapping(g, x))[i] = x[g[f[i]]] なので、Mapping の積としては g∘f になる
    static constexpr F composition(const F& f, const F& g) { return monoid::Mapping<N>::op(g, f); }
    static constexpr F id() { return monoid::Mapping<N>::e(); }
    // 成分 i と j を入れ替える作用素
    static constexpr F swap(long long i, long long j) { return monoid::Mapping<N>::swap(i, j); }
};

}  // namespace acted_monoid

/**
 * @brief 関数オブジェクトから実行時に作る作用素付きモノイド
 * ラムダ式を使える
 *
 * @tparam T 要素の型
 * @tparam Op 積の関数オブジェクトの型
 * @tparam U 作用素の型
 * @tparam Mapping 作用素を適用する関数オブジェクトの型 引数は (f, x), (f, x, sz), (f, x, l, r) のいずれか
 * @tparam Composition 作用素の合成の関数オブジェクトの型
 */
template <typename T, typename Op, typename U, typename Mapping, typename Composition>
struct ActedMonoid {
    using S = T;
    using F = U;
    Op op_object;
    S identity;
    Mapping mapping_object;
    Composition composition_object;
    F lazy_identity;
    ActedMonoid() = default;
    constexpr ActedMonoid(Op op, const S& e, Mapping mapping, Composition composition, const F& id)
        : op_object(std::move(op)), identity(e), mapping_object(std::move(mapping)), composition_object(std::move(composition)), lazy_identity(id) {}
    constexpr S op(const S& a, const S& b) const { return op_object(a, b); }
    constexpr S e() const { return identity; }
    constexpr auto mapping(const F& f, const S& x, long long l, long long r) const {
        if constexpr (std::is_invocable_v<const Mapping&, const F&, const S&, long long, long long>) {
            return mapping_object(f, x, l, r);
        } else if constexpr (std::is_invocable_v<const Mapping&, const F&, const S&, long long>) {
            return mapping_object(f, x, r - l);
        } else {
            return mapping_object(f, x);
        }
    }
    constexpr F composition(const F& f, const F& g) const { return composition_object(f, g); }
    constexpr F id() const { return lazy_identity; }
};
template <typename Op, typename T, typename Mapping, typename Composition, typename U>
ActedMonoid(Op, T, Mapping, Composition, U) -> ActedMonoid<T, Op, U, Mapping, Composition>;
