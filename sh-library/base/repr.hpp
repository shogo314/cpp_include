#pragma once
#include <array>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "traits.hpp"

namespace detail {

///////////////////////////////////////////////////////////////////
/////////////////////////// declaration ///////////////////////////
///////////////////////////////////////////////////////////////////

inline std::string type_str(const char*);
inline std::string initializer_str(const char*);
inline std::string repr(const char*);

inline std::string type_str(const char&);
inline std::string initializer_str(const char&);
inline std::string repr(const char&);

inline std::string type_str(const int&);
inline std::string initializer_str(const int&);
inline std::string repr(const int&);

inline std::string type_str(const long&);
inline std::string initializer_str(const long&);
inline std::string repr(const long&);

inline std::string type_str(const long long&);
inline std::string initializer_str(const long long&);
inline std::string repr(const long long&);

inline std::string type_str(const unsigned int&);
inline std::string initializer_str(const unsigned int&);
inline std::string repr(const unsigned int&);

inline std::string type_str(const unsigned long&);
inline std::string initializer_str(const unsigned long&);
inline std::string repr(const unsigned long&);

inline std::string type_str(const unsigned long long&);
inline std::string initializer_str(const unsigned long long&);
inline std::string repr(const unsigned long long&);

inline std::string type_str(const std::string&);
inline std::string initializer_str(const std::string&);
inline std::string repr(const std::string&);

inline std::string type_str(const float&);
inline std::string initializer_str(const float&);
inline std::string repr(const float&);

inline std::string type_str(const double&);
inline std::string initializer_str(const double&);
inline std::string repr(const double&);

inline std::string type_str(const long double&);
inline std::string initializer_str(const long double&);
inline std::string repr(const long double&);

template <typename T>
inline std::string type_str(const std::vector<T>&);
template <typename T>
inline std::string initializer_str(const std::vector<T>&);
template <typename T>
inline std::string repr(const std::vector<T>&);

template <typename T>
inline std::string type_str(const std::set<T>&);
template <typename T>
inline std::string initializer_str(const std::set<T>&);
template <typename T>
inline std::string repr(const std::set<T>&);

template <typename T1, typename T2>
inline std::string type_str(const std::pair<T1, T2>&);
template <typename T1, typename T2>
inline std::string initializer_str(const std::pair<T1, T2>&);
template <typename T1, typename T2>
inline std::string repr(const std::pair<T1, T2>&);

template <typename T1, typename T2>
inline std::string type_str(const std::map<T1, T2>&);
template <typename T1, typename T2>
inline std::string initializer_str(const std::map<T1, T2>&);
template <typename T1, typename T2>
inline std::string repr(const std::map<T1, T2>&);

template <typename T, size_t N>
inline std::string type_str(const std::array<T, N>&);
template <typename T, size_t N>
inline std::string initializer_str(const std::array<T, N>&);
template <typename T, size_t N>
inline std::string repr(const std::array<T, N>&);

///////////////////////////////////////////////////////////////////
/////////////////////////// definition ////////////////////////////
///////////////////////////////////////////////////////////////////

template <typename T, std::enable_if_t<not has_type_str_v<T>, std::nullptr_t> = nullptr>
inline std::string type_str(const T& t) {
    return "T";
}
template <typename T, std::enable_if_t<has_type_str_v<T>, std::nullptr_t> = nullptr>
inline std::string type_str(const T& t) {
    return t.type_str();
}
template <typename T, std::enable_if_t<not has_initializer_str_v<T>, std::nullptr_t> = nullptr>
inline std::string initializer_str(const T& t) {
    return "{}";
}
template <typename T, std::enable_if_t<has_initializer_str_v<T>, std::nullptr_t> = nullptr>
inline std::string initializer_str(const T& t) {
    return t.initializer_str();
}
template <typename T, std::enable_if_t<not has_repr_v<T>, std::nullptr_t> = nullptr>
inline std::string repr(const T& t) {
    return detail::type_str(t) + detail::initializer_str(t);
}
template <typename T, std::enable_if_t<has_repr_v<T>, std::nullptr_t> = nullptr>
inline std::string repr(const T& t) {
    return t.repr();
}

inline std::string type_str(const char*) {
    return "char*";
}
inline std::string initializer_str(const char* t) {
    return "\"" + std::string(t) + "\"";
}
inline std::string repr(const char* t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const char&) {
    return "char";
}
inline std::string initializer_str(const char& t) {
    std::vector<std::string> v(128, "");
    v['\0'] = "\\0";
    v['\a'] = "\\a";
    v['\b'] = "\\b";
    v['\f'] = "\\f";
    v['\r'] = "\\r";
    v['\n'] = "\\n";
    v['\v'] = "\\v";
    v['\''] = "\\'";
    v['\\'] = "\\\\";
    if (!v[t].empty())
        return "'" + v[t] + "'";
    if (32 <= t and t <= 126)
        return "'" + std::string{t} + "'";
    return "'\\" + std::to_string(t / 64) + std::to_string(t / 8 % 8) + std::to_string(t % 8) + "'";
}
inline std::string repr(const char& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const int&) {
    return "int";
}
inline std::string initializer_str(const int& t) {
    return std::to_string(t);
}
inline std::string repr(const int& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const long&) {
    return "long";
}
inline std::string initializer_str(const long& t) {
    return std::to_string(t) + "l";
}
inline std::string repr(const long& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const long long&) {
    return "long long";
}
inline std::string initializer_str(const long long& t) {
    return std::to_string(t) + "ll";
}
inline std::string repr(const long long& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const unsigned int&) {
    return "unsigned int";
}
inline std::string initializer_str(const unsigned int& t) {
    return std::to_string(t) + "u";
}
inline std::string repr(const unsigned int& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const unsigned long&) {
    return "unsigned long";
}
inline std::string initializer_str(const unsigned long& t) {
    return std::to_string(t) + "ul";
}
inline std::string repr(const unsigned long& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const unsigned long long&) {
    return "unsigned long long";
}
inline std::string initializer_str(const unsigned long long& t) {
    return std::to_string(t) + "ull";
}
inline std::string repr(const unsigned long long& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const std::string&) {
    return "std::string";
}
inline std::string initializer_str(const std::string& t) {
    return "\"" + t + "\"";
}
inline std::string repr(const std::string& t) {
    return detail::type_str(t) + detail::initializer_str(t);
}

inline std::string type_str(const float&) {
    return "float";
}
inline std::string initializer_str(const float& t) {
    return std::to_string(t) + "f";
}
inline std::string repr(const float& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const double&) {
    return "double";
}
inline std::string initializer_str(const double& t) {
    return std::to_string(t);
}
inline std::string repr(const double& t) {
    return detail::initializer_str(t);
}

inline std::string type_str(const long double&) {
    return "long double";
}
inline std::string initializer_str(const long double& t) {
    return std::to_string(t) + "l";
}
inline std::string repr(const long double& t) {
    return detail::initializer_str(t);
}

template <typename T>
inline std::string type_str(const std::vector<T>&) {
    T t;
    return "std::vector<" + detail::type_str(t) + ">";
}
template <typename T>
inline std::string initializer_str(const std::vector<T>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += detail::initializer_str(*itr);
    }
    ret += "}";
    return ret;
}
template <typename T>
inline std::string repr(const std::vector<T>& t) {
    return detail::type_str(t) + detail::initializer_str(t);
}

template <typename T>
inline std::string type_str(const std::set<T>&) {
    T t;
    return "std::set<" + detail::type_str(t) + ">";
}
template <typename T>
inline std::string initializer_str(const std::set<T>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += detail::initializer_str(*itr);
    }
    ret += "}";
    return ret;
}
template <typename T>
inline std::string repr(const std::set<T>& t) {
    return detail::type_str(t) + detail::initializer_str(t);
}

template <typename T1, typename T2>
inline std::string type_str(const std::pair<T1, T2>&) {
    T1 t1;
    T2 t2;
    return "std::pair<" + detail::type_str(t1) + ", " + detail::type_str(t2) + ">";
}
template <typename T1, typename T2>
inline std::string initializer_str(const std::pair<T1, T2>& t) {
    return "{" + detail::repr(t.first) + ", " + detail::repr(t.second) + "}";
}
template <typename T1, typename T2>
inline std::string repr(const std::pair<T1, T2>& t) {
    return detail::type_str(t) + detail::initializer_str(t);
}

template <typename T1, typename T2>
inline std::string type_str(const std::map<T1, T2>&) {
    T1 t1;
    T2 t2;
    return "std::map<" + detail::type_str(t1) + ", " + detail::type_str(t2) + ">";
}
template <typename T1, typename T2>
inline std::string initializer_str(const std::map<T1, T2>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += "{";
        ret += detail::initializer_str(itr->first);
        ret += ", ";
        ret += detail::initializer_str(itr->second);
        ret += "}";
    }
    ret += "}";
    return ret;
}
template <typename T1, typename T2>
inline std::string repr(const std::map<T1, T2>& t) {
    return detail::type_str(t) + detail::initializer_str(t);
}

template <typename T, size_t N>
inline std::string type_str(const std::array<T, N>&) {
    T t;
    return "std::array<" + detail::type_str(t) + ", " + std::to_string(N) + ">";
}
template <typename T, size_t N>
inline std::string initializer_str(const std::array<T, N>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += detail::initializer_str(*itr);
    }
    ret += "}";
    return ret;
}
template <typename T, size_t N>
inline std::string repr(const std::array<T, N>& t) {
    return detail::type_str(t) + detail::initializer_str(t);
}

}  // namespace detail

template <typename T>
inline std::string type_str(const T& t) {
    return detail::type_str(t);
}
template <typename T>
inline std::string initializer_str(const T& t) {
    return detail::initializer_str(t);
}
template <typename T>
inline std::string repr(const T& t) {
    return detail::repr(t);
}
