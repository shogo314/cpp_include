#pragma once

#include <type_traits>

template <typename T1, typename T2>
using signed_common_type_t = std::make_signed_t<std::common_type_t<T1, T2>>;

// template <class MemfunType>
// struct _has_sum {
//    private:
//     static std::false_type confirm(...);
//     template <class U>
//     static auto confirm(U u) -> decltype(static_cast<MemfunType>(&U::sum), std::true_type());

//    public:
//     static constexpr bool value = decltype(confirm(std::declval<rinse::owner_t<MemfunType> >()))::value;
// };
// template <class MemfunType>
// struct has_sum
//     : rinse::meta_bool<_has_sum<MemfunType>::value> {};

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
