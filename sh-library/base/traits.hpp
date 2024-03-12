#pragma once
#include <cstddef>
#include <type_traits>

#define HAS_METHOD(func_name)                                                              \
    namespace detail {                                                                     \
    template <class T, class = void>                                                       \
    struct has_##func_name##_impl : std::false_type {};                                    \
    template <class T>                                                                     \
    struct has_##func_name##_impl<T, std::void_t<decltype(std::declval<T>().func_name())>> \
        : std::true_type {};                                                               \
    }                                                                                      \
    template <class T>                                                                     \
    struct has_##func_name : detail::has_##func_name##_impl<T>::type {};                   \
    template <class T>                                                                     \
    inline constexpr bool has_##func_name##_v = has_##func_name<T>::value;

#define HAS_METHOD_ARG(func_name)                                                                              \
    namespace detail {                                                                                         \
    template <class T, typename U, class = void>                                                               \
    struct has_##func_name##_impl : std::false_type {};                                                        \
    template <class T, typename U>                                                                             \
    struct has_##func_name##_impl<T, U, std::void_t<decltype(std::declval<T>().func_name(std::declval<U>()))>> \
        : std::true_type {};                                                                                   \
    }                                                                                                          \
    template <class T, typename U>                                                                             \
    struct has_##func_name : detail::has_##func_name##_impl<T, U>::type {};                                    \
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
HAS_METHOD_ARG(count)
HAS_METHOD_ARG(find)
HAS_METHOD_ARG(lower_bound)
HAS_METHOD_ARG(upper_bound)

#define ENABLE_IF_T_IMPL(expr) std::enable_if_t<expr, std::nullptr_t> = nullptr
#define ENABLE_IF_T(...) ENABLE_IF_T_IMPL((__VA_ARGS__))

template <class C>
using add_value_type = typename C::value_type;
