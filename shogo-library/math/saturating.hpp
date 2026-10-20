#pragma once
// https://github.com/shogo314/shogo-library/blob/4897960d828446e1fcee244e3d3e67eb3e7f8064/src/math/saturating.hpp

/**
 * @file saturating.hpp
 * @brief 飽和演算 (結果が型の範囲を超えたら、その型の最大値・最小値に丸める)
 */

#include <cassert>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

#if __cplusplus >= 202002L
#include <concepts>
#endif

namespace internal_saturating {
template <class T>
struct is_character : std::false_type {};
template <>
struct is_character<char> : std::true_type {};
template <>
struct is_character<wchar_t> : std::true_type {};
template <>
struct is_character<char16_t> : std::true_type {};
template <>
struct is_character<char32_t> : std::true_type {};
#ifdef __cpp_char8_t
template <>
struct is_character<char8_t> : std::true_type {};
#endif

// bool と文字型を除く整数型 (C++26 の std::add_sat などと同じ条件)
#if __cplusplus >= 202002L
template <class T>
concept integer = std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool> && !is_character<std::remove_cv_t<T>>::value;

using std::cmp_greater;
using std::cmp_less;
#else
template <class T>
inline constexpr bool integer = std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool> && !is_character<std::remove_cv_t<T>>::value;

// C++20 の std::cmp_less と同じ (符号の有無が違っても値として比べる)
template <class T, class U>
constexpr bool cmp_less(T t, U u) noexcept {
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
        return t < u;
    } else if constexpr (std::is_signed_v<T>) {
        return t < 0 || std::make_unsigned_t<T>(t) < u;
    } else {
        return u >= 0 && t < std::make_unsigned_t<U>(u);
    }
}
template <class T, class U>
constexpr bool cmp_greater(T t, U u) noexcept {
    return cmp_less(u, t);
}
#endif
}  // namespace internal_saturating

// C++20 では requires で、C++17 では戻り値の enable_if で整数型に制限する
#if __cplusplus >= 202002L
#define SATURATING_FUNC(cond, R) requires(cond) inline constexpr R
#else
#define SATURATING_FUNC(cond, R) inline constexpr std::enable_if_t<(cond), R>
#endif

// clang-format は SATURATING_FUNC を戻り値の型と読めず、関数名の前で改行してしまうので整形しない
// clang-format off

/**
 * @brief x + y を T の範囲に丸める
 */
template <class T>
SATURATING_FUNC(internal_saturating::integer<T>, T) saturating_add(T x, T y) noexcept {
#ifdef __cpp_lib_saturation_arithmetic
    return std::add_sat(x, y);
#else
    T res{};
    if (!__builtin_add_overflow(x, y, &res)) return res;
    if constexpr (std::is_signed_v<T>) {
        if (y < 0) return std::numeric_limits<T>::min();
    }
    return std::numeric_limits<T>::max();
#endif
}
/**
 * @brief x - y を T の範囲に丸める
 */
template <class T>
SATURATING_FUNC(internal_saturating::integer<T>, T) saturating_sub(T x, T y) noexcept {
#ifdef __cpp_lib_saturation_arithmetic
    return std::sub_sat(x, y);
#else
    T res{};
    if (!__builtin_sub_overflow(x, y, &res)) return res;
    if constexpr (std::is_signed_v<T>) {
        if (y < 0) return std::numeric_limits<T>::max();
    }
    return std::numeric_limits<T>::min();
#endif
}
/**
 * @brief x * y を T の範囲に丸める
 */
template <class T>
SATURATING_FUNC(internal_saturating::integer<T>, T) saturating_mul(T x, T y) noexcept {
#ifdef __cpp_lib_saturation_arithmetic
    return std::mul_sat(x, y);
#else
    T res{};
    if (!__builtin_mul_overflow(x, y, &res)) return res;
    if constexpr (std::is_signed_v<T>) {
        if ((x < 0) != (y < 0)) return std::numeric_limits<T>::min();
    }
    return std::numeric_limits<T>::max();
#endif
}
/**
 * @brief x / y (0 方向に切り捨て) を T の範囲に丸める (y != 0)
 * あふれるのは符号付きの min / -1 だけで、max を返す
 */
template <class T>
SATURATING_FUNC(internal_saturating::integer<T>, T) saturating_div(T x, T y) noexcept {
    assert(y != 0);
#ifdef __cpp_lib_saturation_arithmetic
    return std::div_sat(x, y);
#else
    if constexpr (std::is_signed_v<T>) {
        if (x == std::numeric_limits<T>::min() && y == -1) return std::numeric_limits<T>::max();
    }
    return x / y;
#endif
}
/**
 * @brief 整数 x を R に変換し、R の範囲に収まらなければ R の最大値・最小値に丸める
 */
template <class R, class T>
SATURATING_FUNC(internal_saturating::integer<R> && internal_saturating::integer<T>, R) saturating_cast(T x) noexcept {
#ifdef __cpp_lib_saturation_arithmetic
    return std::saturate_cast<R>(x);
#else
    if (internal_saturating::cmp_less(x, std::numeric_limits<R>::min())) return std::numeric_limits<R>::min();
    if (internal_saturating::cmp_greater(x, std::numeric_limits<R>::max())) return std::numeric_limits<R>::max();
    return static_cast<R>(x);
#endif
}

// clang-format on

#undef SATURATING_FUNC
