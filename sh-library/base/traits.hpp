#pragma once
#include <type_traits>

#define HAS_METHOD(func_name)                                                                                   \
    namespace detail {                                                                                          \
    template <class T, class = void>                                                                            \
    struct has_##func_name##_impl : std::false_type {};                                                         \
    template <class T>                                                                                          \
    struct has_##func_name##_impl<T, std::void_t<decltype(std::declval<T>().func_name())>> : std::true_type {}; \
    }                                                                                                           \
    template <class T>                                                                                          \
    struct has_##func_name : detail::has_##func_name##_impl<T>::type {};                                        \
    template <class T>                                                                                          \
    inline constexpr bool has_##func_name##_v = has_##func_name<T>::value;

HAS_METHOD(repr)
HAS_METHOD(type_str)
HAS_METHOD(initializer_str)
HAS_METHOD(sum)
HAS_METHOD(reversed)
