#pragma once

#include <type_traits>

template <typename T1, typename T2>
struct arithmetic_common_type {
    using type = std::common_type_t<T1, T2>;
};
template <typename T>
struct arithmetic_common_type<T, T> {
    using type = T;
};
template <>
struct arithmetic_common_type<int, unsigned long long> {
    using type = long long;
};
template <>
struct arithmetic_common_type<unsigned long long, int> {
    using type = long long;
};
template <>
struct arithmetic_common_type<long, unsigned long long> {
    using type = long long;
};
template <>
struct arithmetic_common_type<unsigned long long, long> {
    using type = long long;
};
template <>
struct arithmetic_common_type<long long, unsigned long long> {
    using type = long long;
};
template <>
struct arithmetic_common_type<unsigned long long, long long> {
    using type = long long;
};
template <>
struct arithmetic_common_type<int, unsigned long> {
    using type = long;
};
template <>
struct arithmetic_common_type<unsigned long, int> {
    using type = long;
};
template <>
struct arithmetic_common_type<long, unsigned long> {
    using type = long;
};
template <>
struct arithmetic_common_type<unsigned long, long> {
    using type = long;
};
template <typename T1, typename T2>
using arithmetic_common_type_t = typename arithmetic_common_type<T1, T2>::type;

namespace detail {
template <class T, class = void>
struct has_repr_impl : std::false_type {};
template <class T>
struct has_repr_impl<T, std::void_t<decltype(std::declval<T>().repr())>> : std::true_type {};
}  // namespace detail
template <class T>
struct has_repr : detail::has_repr_impl<T>::type {};
template <class T>
inline constexpr bool has_repr_v = has_repr<T>::value;

namespace detail {
template <class T, class = void>
struct has_sum_impl : std::false_type {};
template <class T>
struct has_sum_impl<T, std::void_t<decltype(std::declval<T>().sum())>> : std::true_type {};
}  // namespace detail
template <class T>
struct has_sum : detail::has_sum_impl<T>::type {};
template <class T>
inline constexpr bool has_sum_v = has_sum<T>::value;

namespace detail {
struct is_addible_with_impl {
    template <class... Args>
    static auto check(...) -> std::bool_constant<false>;

    template <class T, class U>
    static auto check(T*, U*) -> decltype(std::declval<T>() + std::declval<U>(), std::bool_constant<true>{});
};
}  // namespace detail
template <class T, class U>
struct is_addible_with : decltype(detail::is_addible_with_impl::check<T, U>(nullptr, nullptr)) {};
template <class T, class U>
inline constexpr bool is_addible_with_v = is_addible_with<T, U>::value;
