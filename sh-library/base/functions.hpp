#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <type_traits>

#include "traits.hpp"

template <typename T1, typename T2>
inline bool chmin(T1& a, T2 b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}

template <typename T1, typename T2>
inline bool chmax(T1& a, T2 b) {
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
signed_common_type_t<T1, T2> max(const T1& t1, const T2& t2) {
    return std::max<signed_common_type_t<T1, T2>>(t1, t2);
}

template <typename T1, typename T2>
signed_common_type_t<T1, T2> min(const T1& t1, const T2& t2) {
    return std::min<signed_common_type_t<T1, T2>>(t1, t2);
}

using std::abs;
using std::gcd;
using std::max;
using std::max_element;
using std::min;
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
