#pragma once
// https://github.com/shogo314/shogo-library/blob/27998142f20ef4e094add3b53cc9c070db4ddbf8/src/utility/traits.hpp

/**
 * @file traits.hpp
 * @brief 型特性
 */

#include <cstddef>
#include <functional>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>

#if __cplusplus >= 202002L
#include <ranges>
#endif

namespace internal_traits {
using std::begin, std::end;

template <class T, class = void>
struct is_range_impl : std::false_type {};
template <class T>
struct is_range_impl<T, std::void_t<decltype(begin(std::declval<T&>()), end(std::declval<T&>()))>> : std::true_type {};

// サイズ付き範囲から要素を直接構築するコンストラクタで共有する。
#if __cplusplus >= 202002L
template <class R, class S>
concept sized_constructible_range = std::ranges::input_range<R> && std::ranges::sized_range<R> &&
                                    std::is_constructible_v<S, std::ranges::range_reference_t<R>>;

template <class R>
auto range_size(R& r) {
    return std::ranges::size(r);
}
template <class R>
auto range_begin(R& r) {
    return std::ranges::begin(r);
}
template <class R>
using range_value_t = std::ranges::range_value_t<R>;
#else
using std::size;
template <class R>
auto range_size(R& r) -> decltype(size(r)) {
    return size(r);
}
template <class R>
auto range_begin(R& r) -> decltype(begin(r)) {
    return begin(r);
}
template <class R>
using range_value_t = typename std::iterator_traits<decltype(range_begin(std::declval<R&>()))>::value_type;

template <class R, class S, class = void>
struct is_sized_constructible_range : std::false_type {};
template <class R, class S>
struct is_sized_constructible_range<R, S, std::void_t<decltype(range_size(std::declval<R&>())), decltype(++std::declval<decltype(begin(std::declval<R&>()))&>()), decltype(begin(std::declval<R&>()) != end(std::declval<R&>())), decltype(*begin(std::declval<R&>()))>>
    : std::is_constructible<S, decltype(*begin(std::declval<R&>()))> {};
#endif
}  // namespace internal_traits

template <class T>
struct is_range : internal_traits::is_range_impl<T>::type {};
template <class T>
inline constexpr bool is_range_v = is_range<T>::value;

#define HAS_METHOD(func_name)                                                              \
    namespace internal_traits {                                                            \
    template <class T, class = void>                                                       \
    struct has_##func_name##_impl : std::false_type {};                                    \
    template <class T>                                                                     \
    struct has_##func_name##_impl<T, std::void_t<decltype(std::declval<T>().func_name())>> \
        : std::true_type {};                                                               \
    }                                                                                      \
    template <class T>                                                                     \
    struct has_##func_name : internal_traits::has_##func_name##_impl<T>::type {};          \
    template <class T>                                                                     \
    inline constexpr bool has_##func_name##_v = has_##func_name<T>::value;

#define HAS_METHOD_ARG(func_name)                                                                              \
    namespace internal_traits {                                                                                \
    template <class T, typename U, class = void>                                                               \
    struct has_##func_name##_impl : std::false_type {};                                                        \
    template <class T, typename U>                                                                             \
    struct has_##func_name##_impl<T, U, std::void_t<decltype(std::declval<T>().func_name(std::declval<U>()))>> \
        : std::true_type {};                                                                                   \
    }                                                                                                          \
    template <class T, typename U>                                                                             \
    struct has_##func_name : internal_traits::has_##func_name##_impl<T, U>::type {};                           \
    template <class T, typename U>                                                                             \
    inline constexpr bool has_##func_name##_v = has_##func_name<T, U>::value;

HAS_METHOD(repr)
HAS_METHOD(type_str)
HAS_METHOD(initializer_str)
HAS_METHOD(max)
HAS_METHOD(min)
HAS_METHOD(reversed)
HAS_METHOD(sorted)
HAS_METHOD(sum)
HAS_METHOD(product)
HAS_METHOD(product_xor)
HAS_METHOD_ARG(count)
HAS_METHOD_ARG(find)
HAS_METHOD_ARG(lower_bound)
HAS_METHOD_ARG(upper_bound)

#define ENABLE_IF_T_IMPL(expr) std::enable_if_t<expr, std::nullptr_t> = nullptr
#define ENABLE_IF_T(...) ENABLE_IF_T_IMPL((__VA_ARGS__))

namespace internal_traits {
// 射影 (proj) の既定値
#if __cplusplus >= 202002L
using identity = std::identity;
#else
struct identity {
    using is_transparent = void;
    template <class T>
    constexpr T&& operator()(T&& x) const noexcept {
        return std::forward<T>(x);
    }
};
#endif
}  // namespace internal_traits

template <class C>
using mem_value_type = typename C::value_type;
template <class C>
using mem_difference_type = typename C::difference_type;

namespace internal_traits {
template <class T, class = void>
struct is_sorted_container_impl : std::false_type {};
template <class T>
struct is_sorted_container_impl<std::set<T>> : std::true_type {};
template <class T>
struct is_sorted_container_impl<std::multiset<T>> : std::true_type {};
}  // namespace internal_traits
template <class T>
struct is_sorted_container : internal_traits::is_sorted_container_impl<T>::type {};
template <class T>
inline constexpr bool is_sorted_container_v = is_sorted_container<T>::value;