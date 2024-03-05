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
template <class T, class = void>
struct has_reversed_impl : std::false_type {};
template <class T>
struct has_reversed_impl<T, std::void_t<decltype(std::declval<T>().reversed())>> : std::true_type {};
}  // namespace detail
template <class T>
struct has_reversed : detail::has_reversed_impl<T>::type {};
template <class T>
inline constexpr bool has_reversed_v = has_reversed<T>::value;
