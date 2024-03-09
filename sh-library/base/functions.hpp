#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <numeric>
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

template <class C, typename T>
inline bool contains(const C& c, const T& t) {
    return c.find(t) != c.end();
}

inline constexpr long long max(const long long& t1, const long long& t2) {
    return std::max<long long>(t1, t2);
}

inline constexpr long long min(const long long& t1, const long long& t2) {
    return std::min<long long>(t1, t2);
}

using std::abs;
using std::gcd;
using std::max_element;
using std::min_element;
using std::size;

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

#define METHOD_EXPAND(func_name)                                             \
    template <typename T, ENABLE_IF_T(has_##func_name##_v<T>)>               \
    inline constexpr auto func_name(const T& t) -> decltype(t.func_name()) { \
        return t.func_name();                                                \
    }

METHOD_EXPAND(sum)
METHOD_EXPAND(product)
METHOD_EXPAND(reversed)

template <typename T, ENABLE_IF_T(not has_reversed_v<T>)>
inline constexpr T reversed(T t) {
    using namespace std;
    reverse(t.begin(), t.end());
    return t;
}

template <typename T>
inline constexpr T sorted(T t) {
    using namespace std;
    sort(t.begin(), t.end());
    return t;
}

#define METHOD_AND_FUNC_ARG_EXPAND(func_name)                                     \
    template <typename T, typename U, ENABLE_IF_T(has_##func_name##_v<T, U>)>     \
    inline constexpr auto func_name(const T& t, const U& u)                       \
        -> decltype(t.func_name(u)) {                                             \
        return t.func_name(u);                                                    \
    }                                                                             \
    template <typename T, typename U, ENABLE_IF_T(not has_##func_name##_v<T, U>)> \
    inline constexpr auto func_name(const T& t, const U& u)                       \
        -> decltype(func_name(t.begin(), t.end(), u)) {                           \
        using namespace std;                                                      \
        return func_name(t.begin(), t.end(), u);                                  \
    }

METHOD_AND_FUNC_ARG_EXPAND(count)
METHOD_AND_FUNC_ARG_EXPAND(find)
METHOD_AND_FUNC_ARG_EXPAND(lower_bound)
METHOD_AND_FUNC_ARG_EXPAND(upper_bound)
