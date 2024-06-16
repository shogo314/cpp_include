#pragma once
#include <algorithm>
#include <cassert>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include "traits.hpp"

#define METHOD_EXPAND(func)                                        \
    template <typename T, ENABLE_IF_T(has_##func##_v<T>)>          \
    inline constexpr auto func(const T &t) -> decltype(t.func()) { \
        return t.func();                                           \
    }

#define METHOD_AND_FUNC_ARG_EXPAND(func)                                     \
    template <typename T, typename U, ENABLE_IF_T(has_##func##_v<T, U>)>     \
    inline constexpr auto func(const T &t, const U &u)                       \
        -> decltype(t.func(u)) {                                             \
        return t.func(u);                                                    \
    }                                                                        \
    template <typename T, typename U, ENABLE_IF_T(not has_##func##_v<T, U>)> \
    inline constexpr auto func(const T &t, const U &u)                       \
        -> decltype(std::func(t.begin(), t.end(), u)) {                      \
        return std::func(t.begin(), t.end(), u);                             \
    }

METHOD_EXPAND(reversed)
template <class C, ENABLE_IF_T(not has_reversed_v<C>)>
inline constexpr C reversed(C t) {
    std::reverse(t.begin(), t.end());
    return t;
}

METHOD_EXPAND(sorted)
template <class C, ENABLE_IF_T(not has_sorted_v<C>)>
inline constexpr C sorted(C t, bool reverse = false) {
    std::sort(t.begin(), t.end());
    if (reverse) std::reverse(t.begin(), t.end());
    return t;
}
template <class C, class F, ENABLE_IF_T(not has_sorted_v<C> and std::is_invocable_r_v<bool, F, mem_value_type<C>, mem_value_type<C>>)>
inline constexpr C sorted(C t, F f) {
    std::sort(t.begin(), t.end(), f);
    return t;
}

template <class C>
inline constexpr void sort(C &t, bool reverse = false) {
    std::sort(t.begin(), t.end());
    if (reverse) std::reverse(t.begin(), t.end());
}
template <class C, class F, ENABLE_IF_T(std::is_invocable_r_v<bool, F, mem_value_type<C>, mem_value_type<C>>)>
inline constexpr void sort(C &t, F f) {
    std::sort(t.begin(), t.end(), f);
}
template <class C, class F, ENABLE_IF_T(std::is_invocable_v<F, mem_value_type<C>>)>
inline constexpr void sort_by_key(C &t, F f) {
    std::sort(t.begin(), t.end(), [&](const mem_value_type<C> &left, const mem_value_type<C> &right) {
        return f(left) < f(right);
    });
}

template <class C>
inline constexpr void reverse(C &t) {
    std::reverse(t.begin(), t.end());
}

METHOD_EXPAND(max)
template <class C, ENABLE_IF_T(not has_max_v<C> and is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> max(const C &v) {
    assert(v.begin() != v.end());
    return *v.rbegin();
}
template <class C, ENABLE_IF_T(not has_max_v<C> and not is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> max(const C &v) {
    assert(v.begin() != v.end());
    return *std::max_element(v.begin(), v.end());
}
template <typename T>
inline constexpr T max(const std::initializer_list<T> &v) {
    return std::max(v);
}

METHOD_EXPAND(min)
template <class C, ENABLE_IF_T(not has_max_v<C> and is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> min(const C &v) {
    assert(v.begin() != v.end());
    return *v.begin();
}
template <class C, ENABLE_IF_T(not has_max_v<C> and not is_sorted_container_v<C>)>
inline constexpr mem_value_type<C> min(const C &v) {
    assert(v.begin() != v.end());
    return *std::min_element(v.begin(), v.end());
}
template <typename T>
inline constexpr T min(const std::initializer_list<T> &v) {
    return std::min(v);
}

METHOD_EXPAND(sum)
template <class C, ENABLE_IF_T(not has_sum_v<C>)>
inline constexpr mem_value_type<C> sum(const C &v) {
    return std::accumulate(v.begin(), v.end(), mem_value_type<C>{});
}
template <typename T>
inline constexpr T sum(const std::initializer_list<T> &v) {
    return std::accumulate(v.begin(), v.end(), T{});
}

METHOD_EXPAND(product)
template <class C, ENABLE_IF_T(not has_product_v<C>)>
inline constexpr mem_value_type<C> product(const C &v) {
    return std::accumulate(v.begin(), v.end(), mem_value_type<C>{1}, std::multiplies<mem_value_type<C>>());
}
template <typename T>
inline constexpr T product(const std::initializer_list<T> &v) {
    return std::accumulate(v.begin(), v.end(), T{1}, std::multiplies<T>());
}

METHOD_EXPAND(product_xor)
template <class C, ENABLE_IF_T(not has_product_xor_v<C>)>
inline constexpr mem_value_type<C> product_xor(const C &v) {
    return std::accumulate(v.begin(), v.end(), mem_value_type<C>{0}, std::bit_xor<mem_value_type<C>>());
}
template <typename T>
inline constexpr T product_xor(const std::initializer_list<T> &v) {
    return std::accumulate(v.begin(), v.end(), T{0}, std::bit_xor<T>());
}

template <class C>
inline constexpr mem_value_type<C> maximum_subarray(const C &v) {
    assert(not v.empty());
    auto itr = v.begin();
    mem_value_type<C> tmp = *itr++;
    mem_value_type<C> res = tmp;
    while (itr != v.end()) {
        tmp += *itr;
        if (tmp < *itr) tmp = *itr;
        if (res < tmp) res = tmp;
        ++itr;
    }
    return res;
}
template <class C>
inline constexpr mem_value_type<C> maximum_subarray(const C &v, mem_value_type<C> init) {
    mem_value_type<C> res = init, tmp = init;
    for (const auto &a : v) {
        tmp += a;
        if (tmp < init) tmp = init;
        if (res < tmp) res = tmp;
    }
    return res;
}

METHOD_AND_FUNC_ARG_EXPAND(count)
METHOD_AND_FUNC_ARG_EXPAND(find)
METHOD_AND_FUNC_ARG_EXPAND(lower_bound)
METHOD_AND_FUNC_ARG_EXPAND(upper_bound)

template <class C, typename T>
inline constexpr bool contains(const C &c, const T &t) {
    return find(c, t) != c.end();
}

template <class C>
inline constexpr mem_value_type<C> gcd(const C &v) {
    mem_value_type<C> init(0);
    for (const auto &e : v) init = std::gcd(init, e);
    return init;
}

template <class C>
inline constexpr mem_value_type<C> average(const C &v) {
    assert(v.size());
    return sum(v) / v.size();
}

template <class C>
inline constexpr mem_value_type<C> median(const C &v) {
    assert(not v.empty());
    std::vector<size_t> u(v.size());
    std::iota(u.begin(), u.end(), 0);
    std::sort(u.begin(), u.end(), [&](size_t a, size_t b) {
        return v[a] < v[b];
    });
    if (v.size() & 1) {
        return v[u[v.size() / 2]];
    }
    // C++20
    // return std::midpoint(v[u[v.size() / 2]], v[u[v.size() / 2 - 1]]);
    return (v[u[v.size() / 2]] + v[u[v.size() / 2 - 1]]) / 2;
}

template <class C, typename U>
inline constexpr std::ptrdiff_t index(const C &v, const U &x) {
    return std::distance(v.begin(), find(v, x));
}

template <class C, ENABLE_IF_T(std::is_integral_v<mem_value_type<C>>)>
inline constexpr mem_value_type<C> mex(const C &v) {
    std::vector<bool> b(v.size() + 1);
    for (const auto &a : v) {
        if (0 <= a and a < b.size()) {
            b[a] = true;
        }
    }
    mem_value_type<C> ret;
    for (size_t i = 0; i < b.size(); i++) {
        if (not b[i]) {
            ret = i;
            break;
        }
    }
    return ret;
}

template <class C>
inline constexpr mem_difference_type<C> bisect_left(const C &v, const mem_value_type<C> &x) {
    return std::distance(v.begin(), lower_bound(v, x));
}
template <class C>
inline constexpr mem_difference_type<C> bisect_right(const C &v, const mem_value_type<C> &x) {
    return std::distance(v.begin(), upper_bound(v, x));
}
