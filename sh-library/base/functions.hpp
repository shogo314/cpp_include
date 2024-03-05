#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <type_traits>
#include "traits.hpp"

template <typename T1, typename T2>
inline constexpr bool chmin(T1& a, T2 b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}

template <typename T1, typename T2>
inline constexpr bool chmax(T1& a, T2 b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}

using std::size;

template <class C, typename T>
inline bool contains(const C& c, const T& t) {
    return c.find(t) != c.end();
}

template <typename T1, typename T2>
constexpr arithmetic_common_type_t<T1, T2> max(const T1& t1, const T2& t2) {
    return std::max<arithmetic_common_type_t<T1, T2>>(t1, t2);
}

template <typename T1, typename T2>
constexpr arithmetic_common_type_t<T1, T2> min(const T1& t1, const T2& t2) {
    return std::min<arithmetic_common_type_t<T1, T2>>(t1, t2);
}

using std::abs;
using std::gcd;
using std::max_element;
using std::min_element;

template <typename T>
constexpr T extgcd(const T& a, const T& b, T& x, T& y) {
    T d = a;
    if (b != 0) {
        d = extgcd(b, a % b, y, x);
        y -= (a / b) * x;
    } else {
        x = 1;
        y = 0;
    }
    return d;
}

template <typename T, std::enable_if_t<has_sum_v<T>, std::nullptr_t> = nullptr>
constexpr auto sum(const T& t) -> decltype(t.sum()) {
    return t.sum();
}

template <typename T, std::enable_if_t<has_reversed_v<T>, std::nullptr_t> = nullptr>
constexpr auto reversed(const T& t) -> decltype(t.reversed()) {
    return t.reversed();
}

template <typename T, std::enable_if_t<not has_reversed_v<T>, std::nullptr_t> = nullptr>
constexpr T reversed(T t) {
    std::reverse(t.begin(), t.end());
    return t;
}
