#pragma once

/**
 * @file container_func.hpp
 * @brief コンテナに対する関数 (sorted, max, sum など)
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <vector>

#include "traits.hpp"
// https://github.com/shogo314/shogo-library/blob/8eadfabbff53ac04c3000e8f0585b08e4fe5155f/src/utility/container_func.hpp

#define METHOD_EXPAND(func)                                        \
    template <typename T, ENABLE_IF_T(has_##func##_v<T>)>          \
    inline constexpr auto func(const T& t) -> decltype(t.func()) { \
        return t.func();                                           \
    }

#define METHOD_AND_FUNC_ARG_EXPAND(func)                                  \
    template <typename T, typename U, ENABLE_IF_T(has_##func##_v<T, U>)>  \
    inline constexpr auto func(const T& t, const U& u)                    \
        -> decltype(t.func(u)) {                                          \
        return t.func(u);                                                 \
    }                                                                     \
    template <typename T, typename U, ENABLE_IF_T(!has_##func##_v<T, U>)> \
    inline constexpr auto func(const T& t, const U& u)                    \
        -> decltype(std::func(t.begin(), t.end(), u)) {                   \
        return std::func(t.begin(), t.end(), u);                          \
    }

METHOD_EXPAND(reversed)
template <class C, ENABLE_IF_T(!has_reversed_v<C>)>
inline constexpr C reversed(C t) {
    std::reverse(t.begin(), t.end());
    return t;
}

namespace internal_container_func {
// comp(proj(a), proj(b)) で比べられるか
template <class C, class F, class P, class = void>
struct is_projected_comp : std::false_type {};
template <class C, class F, class P>
struct is_projected_comp<C, F, P, std::void_t<std::invoke_result_t<P&, const mem_value_type<C>&>>>
    : std::is_invocable_r<bool, F&, std::invoke_result_t<P&, const mem_value_type<C>&>, std::invoke_result_t<P&, const mem_value_type<C>&>> {};
}  // namespace internal_container_func

METHOD_EXPAND(sorted)
template <class C, class F = std::less<>, class P = internal_traits::identity, ENABLE_IF_T(!has_sorted_v<C> && internal_container_func::is_projected_comp<C, F, P>::value)>
inline constexpr C sorted(C t, F comp = F(), P proj = P()) {
    std::sort(t.begin(), t.end(), [&](const mem_value_type<C>& a, const mem_value_type<C>& b) {
        return std::invoke(comp, std::invoke(proj, a), std::invoke(proj, b));
    });
    return t;
}

template <class C, class F = std::less<>, class P = internal_traits::identity, ENABLE_IF_T(internal_container_func::is_projected_comp<C, F, P>::value)>
inline constexpr void sort(C& t, F comp = F(), P proj = P()) {
    std::sort(t.begin(), t.end(), [&](const mem_value_type<C>& a, const mem_value_type<C>& b) {
        return std::invoke(comp, std::invoke(proj, a), std::invoke(proj, b));
    });
}
// sort(c, {}, proj) 用。std::vector などでは ADL で std::sort(first, last, comp) も候補になり、
// {} を F で受けると std::sort が選ばれるので、{} が標準変換で渡る std::nullptr_t で受ける
template <class C, class P, ENABLE_IF_T(internal_container_func::is_projected_comp<C, std::less<>, P>::value)>
inline constexpr void sort(C& t, std::nullptr_t, P proj) {
    sort(t, std::less<>(), proj);
}

template <class C>
inline constexpr void reverse(C& t) {
    std::reverse(t.begin(), t.end());
}

METHOD_EXPAND(max)
template <class C, ENABLE_IF_T(!has_max_v<C> && is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> max(const C& v) {
    assert(v.begin() != v.end());
    return *v.rbegin();
}
template <class C, ENABLE_IF_T(!has_max_v<C> && !is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> max(const C& v) {
    assert(v.begin() != v.end());
    return *std::max_element(v.begin(), v.end());
}
template <typename T>
inline constexpr T max(const std::initializer_list<T>& v) {
    return std::max(v);
}

METHOD_EXPAND(min)
template <class C, ENABLE_IF_T(!has_min_v<C> && is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> min(const C& v) {
    assert(v.begin() != v.end());
    return *v.begin();
}
template <class C, ENABLE_IF_T(!has_min_v<C> && !is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> min(const C& v) {
    assert(v.begin() != v.end());
    return *std::min_element(v.begin(), v.end());
}
template <typename T>
inline constexpr T min(const std::initializer_list<T>& v) {
    return std::min(v);
}

METHOD_EXPAND(sum)
template <class C, ENABLE_IF_T(!has_sum_v<C>)>
inline constexpr mem_value_type<C> sum(const C& v) {
    return std::accumulate(v.begin(), v.end(), mem_value_type<C>{});
}
template <typename T>
inline constexpr T sum(const std::initializer_list<T>& v) {
    return std::accumulate(v.begin(), v.end(), T{});
}

METHOD_EXPAND(product)
template <class C, ENABLE_IF_T(!has_product_v<C>)>
inline constexpr mem_value_type<C> product(const C& v) {
    return std::accumulate(v.begin(), v.end(), mem_value_type<C>{1}, std::multiplies<mem_value_type<C>>());
}
template <typename T>
inline constexpr T product(const std::initializer_list<T>& v) {
    return std::accumulate(v.begin(), v.end(), T{1}, std::multiplies<T>());
}

METHOD_EXPAND(product_xor)
template <class C, ENABLE_IF_T(!has_product_xor_v<C>)>
inline constexpr mem_value_type<C> product_xor(const C& v) {
    return std::accumulate(v.begin(), v.end(), mem_value_type<C>{0}, std::bit_xor<mem_value_type<C>>());
}
template <typename T>
inline constexpr T product_xor(const std::initializer_list<T>& v) {
    return std::accumulate(v.begin(), v.end(), T{0}, std::bit_xor<T>());
}

METHOD_AND_FUNC_ARG_EXPAND(count)
METHOD_AND_FUNC_ARG_EXPAND(find)
METHOD_AND_FUNC_ARG_EXPAND(lower_bound)
METHOD_AND_FUNC_ARG_EXPAND(upper_bound)

template <class C, typename T>
inline constexpr bool contains(const C& c, const T& t) {
    return find(c, t) != c.end();
}

template <class C>
inline constexpr mem_value_type<C> gcd(const C& v) {
    mem_value_type<C> init(0);
    for (const auto& e : v) init = std::gcd(init, e);
    return init;
}

template <class C>
inline constexpr mem_value_type<C> average(const C& v) {
    assert(v.size());
    // v.size() は符号なしなので、そのまま割ると負の和が符号なしに変換されて壊れる
    return sum(v) / static_cast<mem_value_type<C>>(v.size());
}

template <class C>
inline constexpr mem_value_type<C> median(const C& v) {
    assert(!v.empty());
    std::vector<std::size_t> u(v.size());
    std::iota(u.begin(), u.end(), 0);
    std::sort(u.begin(), u.end(), [&](std::size_t a, std::size_t b) {
        return v[a] < v[b];
    });
    if (v.size() & 1) {
        return v[u[v.size() / 2]];
    }
    // C++20
    // return std::midpoint(v[u[v.size() / 2]], v[u[v.size() / 2 - 1]]);
    return (v[u[v.size() / 2]] + v[u[v.size() / 2 - 1]]) / 2;
}

// x が最初に現れる位置。なければ -1
template <class C, typename U>
inline constexpr long long index(const C& v, const U& x) {
    auto it = find(v, x);
    if (it == v.end()) return -1;
    return static_cast<long long>(std::distance(v.begin(), it));
}

// f(x) が true になる最初の位置。なければ -1
template <class C, class F>
inline constexpr long long index_if(const C& v, F f) {
    long long i = 0;
    for (const auto& e : v) {
        if (f(e)) return i;
        i++;
    }
    return -1;
}

// f(x) が true になる位置を昇順に並べたもの
template <class C, class F>
inline std::vector<long long> indices_if(const C& v, F f) {
    std::vector<long long> res;
    long long i = 0;
    for (const auto& e : v) {
        if (f(e)) res.push_back(i);
        i++;
    }
    return res;
}

// f(x) が true になる要素の個数
template <class C, class F>
inline constexpr long long count_if(const C& v, F f) {
    long long res = 0;
    for (const auto& e : v) {
        if (f(e)) res++;
    }
    return res;
}

/**
 * @brief 同じ長さの 2 つの列のハミング距離 (要素が異なる位置の数) O(N)
 *
 * @tparam R1 配列の型
 * @tparam R2 配列の型
 * @param s 配列
 * @param t 配列 (s と同じ長さ)
 * @return long long ハミング距離
 */
template <typename R1, typename R2>
inline constexpr auto hamming_distance(const R1& s, const R2& t) -> decltype(std::begin(s), std::begin(t), 0LL) {
    auto a = std::begin(s);
    auto b = std::begin(t);
    long long n = std::end(s) - a;
    assert(n == std::end(t) - b);
    long long res = 0;
    for (long long i = 0; i < n; i++) {
        if (!(a[i] == b[i])) res++;
    }
    return res;
}

template <class C, ENABLE_IF_T(std::is_integral_v<mem_value_type<C>>)>
inline constexpr mem_value_type<C> mex(const C& v) {
    std::vector<bool> b(v.size() + 1);
    for (const auto& a : v) {
        if (0 <= a && static_cast<std::size_t>(a) < b.size()) {
            b[a] = true;
        }
    }
    mem_value_type<C> ret = 0;  // b の要素数は v より 1 多いので、下のループで必ず見つかる
    for (std::size_t i = 0; i < b.size(); i++) {
        if (!b[i]) {
            ret = i;
            break;
        }
    }
    return ret;
}

template <class C>
inline constexpr long long bisect_left(const C& v, const mem_value_type<C>& x) {
    return static_cast<long long>(std::distance(v.begin(), lower_bound(v, x)));
}
template <class C>
inline constexpr long long bisect_right(const C& v, const mem_value_type<C>& x) {
    return static_cast<long long>(std::distance(v.begin(), upper_bound(v, x)));
}
