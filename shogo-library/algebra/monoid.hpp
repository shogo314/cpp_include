#pragma once
// https://github.com/shogo314/shogo-library/blob/b591e1ab6db09d0025e385b1bb5f4d8982cc4d1f/src/algebra/monoid.hpp

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
#include <array>
#include <cstddef>
#include <istream>
#include <limits>
#include <numeric>
#include <ostream>
#include <type_traits>
#include <utility>

/**
 * @brief 1次関数 f(x) = a * x + b
 *
 * monoid::Composite の要素や acted_monoid::AffineSum の作用素に使う
 * 例: Affine<mint> f{2, 3}; f(x) は 2x + 3。省略すると恒等写像 {1, 0}
 */
template <typename T>
struct Affine {
    T a = T(1);  //!< 傾き
    T b = T(0);  //!< 切片
    constexpr T operator()(const T& x) const { return a * x + b; }
    friend constexpr bool operator==(const Affine& f, const Affine& g) { return f.a == g.a && f.b == g.b; }
    friend constexpr bool operator!=(const Affine& f, const Affine& g) { return !(f == g); }
    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, Affine& f) {
        return is >> f.a >> f.b;
    }
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Affine& f) {
        return os << f.a << CharT(' ') << f.b;
    }
};

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
 * @brief {0, 1, ..., N-1} から自分自身への写像の合成 (変換モノイド)
 *
 * 要素 f は f[x] が x の行き先を表す std::array<int, N>。全単射でなくてよい
 * op(f, g) = f∘g (g を先に適用する)。区間 [l, r) の積は f_l(...f_{r-1}(x)) を表す
 * 左から順に適用したい場合は monoid::Reverse<monoid::Mapping<N>> を使う
 * 例: SegTree<monoid::Mapping<3>> seg(n); seg.set(i, {0, 2, 1});
 * swap(i, j) で i と j を入れ替える写像を作れる
 *
 * @tparam N 写像の定義域・値域の大きさ
 */
template <std::size_t N>
struct Mapping {
    using S = std::array<int, N>;
    static constexpr S op(const S& f, const S& g) {
        S h{};
        for (std::size_t x = 0; x < N; x++) h[x] = f[g[x]];
        return h;
    }
    static constexpr S e() {
        S h{};
        for (std::size_t x = 0; x < N; x++) h[x] = static_cast<int>(x);
        return h;
    }
    // i と j を入れ替え、ほかは動かさない写像 (互換)
    static constexpr S swap(long long i, long long j) {
        S h = e();
        h[i] = static_cast<int>(j);
        h[j] = static_cast<int>(i);
        return h;
    }
};

/**
 * @brief 1次関数 (Affine) の合成
 * op(f, g) = f∘g (g を先に適用する)
 * 区間 [l, r) の積は f_l(...f_{r-1}(x)) を表す
 * 左から順に適用したい (f_{r-1}(...f_l(x)) を求めたい) 場合は monoid::Reverse<monoid::Composite<T>> を使う
 */
template <typename T>
struct Composite {
    using S = Affine<T>;
    static constexpr S op(const S& f, const S& g) { return {f.a * g.a, f.a * g.b + f.b}; }
    static constexpr S e() { return {T(1), T(0)}; }
};

/**
 * @brief 括弧列 対応の取れていない ')' と '(' の個数
 */
struct Parenthesis {
    struct S {
        long long close;  // 対応の取れていない ')' の個数
        long long open;   // 対応の取れていない '(' の個数
        constexpr S(char c = 0) : close(c == ')'), open(c == '(') {}
        constexpr S(long long close, long long open) : close(close), open(open) {}
        /**
         * @brief 正しい括弧列か
         */
        constexpr bool is_valid() const { return close == 0 && open == 0; }
    };
    static constexpr S op(const S& a, const S& b) {
        if (a.open >= b.close) return S(a.close, a.open - b.close + b.open);
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
template <typename T, typename U = long long>
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
template <typename T, typename U = long long>
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
        if (a.first < b.first) return a;
        if (b.first < a.first) return b;
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
        if (a.first < b.first) return b;
        if (b.first < a.first) return a;
        return {a.first, a.second + b.second};
    }
    static constexpr S e() { return {std::numeric_limits<T>::lowest(), U(0)}; }
};

/**
 * @brief 区間の和と、空を含む prefix の和の最大値
 * S(x) で値 x の1要素を表す要素を作る。単位元は空の区間 {0, 0}
 */
template <typename T>
struct PrefixSumMax {
    struct S {
        T sum;  // 区間の和
        T max;  // 空を含む prefix の和の最大値 (0 以上)
        constexpr S() : sum(0), max(0) {}
        constexpr S(const T& x) : sum(x), max(x < T(0) ? T(0) : x) {}
        constexpr S(const T& sum, const T& max) : sum(sum), max(max) {}
        friend constexpr bool operator==(const S& a, const S& b) { return a.sum == b.sum && a.max == b.max; }
        friend constexpr bool operator!=(const S& a, const S& b) { return !(a == b); }
    };
    static constexpr S op(const S& a, const S& b) { return S(a.sum + b.sum, std::max(a.max, a.sum + b.max)); }
    static constexpr S e() { return S(); }
};

/**
 * @brief 区間の和と、空を含む prefix の和の最小値
 * S(x) で値 x の1要素を表す要素を作る。単位元は空の区間 {0, 0}
 */
template <typename T>
struct PrefixSumMin {
    struct S {
        T sum;  // 区間の和
        T min;  // 空を含む prefix の和の最小値 (0 以下)
        constexpr S() : sum(0), min(0) {}
        constexpr S(const T& x) : sum(x), min(T(0) < x ? T(0) : x) {}
        constexpr S(const T& sum, const T& min) : sum(sum), min(min) {}
        friend constexpr bool operator==(const S& a, const S& b) { return a.sum == b.sum && a.min == b.min; }
        friend constexpr bool operator!=(const S& a, const S& b) { return !(a == b); }
    };
    static constexpr S op(const S& a, const S& b) { return S(a.sum + b.sum, std::min(a.min, a.sum + b.min)); }
    static constexpr S e() { return S(); }
};

/**
 * @brief 区間の和と、空を含む suffix の和の最大値
 * S(x) で値 x の1要素を表す要素を作る。単位元は空の区間 {0, 0}
 */
template <typename T>
struct SuffixSumMax {
    struct S {
        T sum;  // 区間の和
        T max;  // 空を含む suffix の和の最大値 (0 以上)
        constexpr S() : sum(0), max(0) {}
        constexpr S(const T& x) : sum(x), max(x < T(0) ? T(0) : x) {}
        constexpr S(const T& sum, const T& max) : sum(sum), max(max) {}
        friend constexpr bool operator==(const S& a, const S& b) { return a.sum == b.sum && a.max == b.max; }
        friend constexpr bool operator!=(const S& a, const S& b) { return !(a == b); }
    };
    static constexpr S op(const S& a, const S& b) { return S(a.sum + b.sum, std::max(b.max, a.max + b.sum)); }
    static constexpr S e() { return S(); }
};

/**
 * @brief 区間の和と、空を含む suffix の和の最小値
 * S(x) で値 x の1要素を表す要素を作る。単位元は空の区間 {0, 0}
 */
template <typename T>
struct SuffixSumMin {
    struct S {
        T sum;  // 区間の和
        T min;  // 空を含む suffix の和の最小値 (0 以下)
        constexpr S() : sum(0), min(0) {}
        constexpr S(const T& x) : sum(x), min(T(0) < x ? T(0) : x) {}
        constexpr S(const T& sum, const T& min) : sum(sum), min(min) {}
        friend constexpr bool operator==(const S& a, const S& b) { return a.sum == b.sum && a.min == b.min; }
        friend constexpr bool operator!=(const S& a, const S& b) { return !(a == b); }
    };
    static constexpr S op(const S& a, const S& b) { return S(a.sum + b.sum, std::min(b.min, a.min + b.sum)); }
    static constexpr S e() { return S(); }
};

/**
 * @brief 区間の和、空を含む prefix・suffix の和の最大値、空を含む連続部分列の和の最大値
 * S(x) で値 x の1要素を表す要素を作る。単位元は空の区間 {0, 0, 0, 0}
 * 静的な列に対して一度だけ求めるなら dp/maximum-subarray.hpp の maximum_subarray
 */
template <typename T>
struct MaxSubarraySum {
    struct S {
        T sum;     // 区間の和
        T prefix;  // 空を含む prefix の和の最大値 (0 以上)
        T suffix;  // 空を含む suffix の和の最大値 (0 以上)
        T best;    // 空を含む連続部分列の和の最大値 (0 以上)
        constexpr S() : sum(0), prefix(0), suffix(0), best(0) {}
        constexpr S(const T& x) : sum(x), prefix(x < T(0) ? T(0) : x), suffix(prefix), best(prefix) {}
        constexpr S(const T& sum, const T& prefix, const T& suffix, const T& best)
            : sum(sum), prefix(prefix), suffix(suffix), best(best) {}
        friend constexpr bool operator==(const S& a, const S& b) {
            return a.sum == b.sum && a.prefix == b.prefix && a.suffix == b.suffix && a.best == b.best;
        }
        friend constexpr bool operator!=(const S& a, const S& b) { return !(a == b); }
    };
    static constexpr S op(const S& a, const S& b) {
        return S(a.sum + b.sum, std::max(a.prefix, a.sum + b.prefix), std::max(b.suffix, a.suffix + b.sum),
                 std::max({a.best, b.best, a.suffix + b.prefix}));
    }
    static constexpr S e() { return S(); }
};

/**
 * @brief 積の順序を逆にしたモノイド op(a, b) = M::op(b, a)
 *
 * 例: SegTree<monoid::Reverse<monoid::Composite<mint>>> seg(v); の区間 [l, r) の積は f_{r-1}(...f_l(x))
 *
 * @tparam M もとのモノイド
 */
template <typename M>
struct Reverse {
    using S = typename M::S;
    M m;
    constexpr Reverse() : m() {}
    constexpr explicit Reverse(M m) : m(std::move(m)) {}
    constexpr S op(const S& a, const S& b) const { return m.op(b, a); }
    constexpr S e() const { return m.e(); }
};

}  // namespace monoid

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
