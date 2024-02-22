#pragma once

#include <functional>
#include <iterator>

#include "sh-library/base/traits.hpp"

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

template <typename T1, typename T2>
inline const add_result_type<T1, T2>& max(const T1& a, const T2& b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

template <typename T1, typename T2>
inline const add_result_type<T1, T2>& min(const T1& a, const T2& b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}
