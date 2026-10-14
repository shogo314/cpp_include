#pragma once

/**
 * @file repr.hpp
 * @brief 出力をそのままコンストラクタに使える文字列化 (repr)
 */

#include <array>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "traits.hpp"
// https://github.com/shogo314/shogo-library/blob/a2fefc7ec36d798ade24522d799381341eac20dd/src/utility/repr.hpp

namespace internal_repr {

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

template <typename T, std::size_t N>
inline std::string type_str(const std::array<T, N>&);
template <typename T, std::size_t N>
inline std::string initializer_str(const std::array<T, N>&);
template <typename T, std::size_t N>
inline std::string repr(const std::array<T, N>&);

///////////////////////////////////////////////////////////////////
/////////////////////////// definition ////////////////////////////
///////////////////////////////////////////////////////////////////

template <typename T, ENABLE_IF_T(!has_type_str_v<T>)>
inline std::string type_str(const T& t) {
    return "T";
}
template <typename T, ENABLE_IF_T(has_type_str_v<T>)>
inline std::string type_str(const T& t) {
    return t.type_str();
}
template <typename T, ENABLE_IF_T(!has_initializer_str_v<T>)>
inline std::string initializer_str(const T& t) {
    return "{}";
}
template <typename T, ENABLE_IF_T(has_initializer_str_v<T>)>
inline std::string initializer_str(const T& t) {
    return t.initializer_str();
}
template <typename T, ENABLE_IF_T(!has_repr_v<T>)>
inline std::string repr(const T& t) {
    return internal_repr::type_str(t) + internal_repr::initializer_str(t);
}
template <typename T, ENABLE_IF_T(has_repr_v<T>)>
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
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const char&) {
    return "char";
}
inline std::string initializer_str(const char& t) {
    // char が符号付きでも添字が負にならないように unsigned char で扱う (UTF-8 のバイトなど)
    const unsigned char u = static_cast<unsigned char>(t);
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
    if (u < 128 && !v[u].empty())
        return "'" + v[u] + "'";
    if (32 <= u && u <= 126)
        return "'" + std::string{t} + "'";
    return "'\\" + std::to_string(u / 64) + std::to_string(u / 8 % 8) + std::to_string(u % 8) + "'";
}
inline std::string repr(const char& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const int&) {
    return "int";
}
inline std::string initializer_str(const int& t) {
    return std::to_string(t);
}
inline std::string repr(const int& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const long&) {
    return "long";
}
inline std::string initializer_str(const long& t) {
    return std::to_string(t) + "l";
}
inline std::string repr(const long& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const long long&) {
    return "long long";
}
inline std::string initializer_str(const long long& t) {
    return std::to_string(t) + "ll";
}
inline std::string repr(const long long& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const unsigned int&) {
    return "unsigned int";
}
inline std::string initializer_str(const unsigned int& t) {
    return std::to_string(t) + "u";
}
inline std::string repr(const unsigned int& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const unsigned long&) {
    return "unsigned long";
}
inline std::string initializer_str(const unsigned long& t) {
    return std::to_string(t) + "ul";
}
inline std::string repr(const unsigned long& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const unsigned long long&) {
    return "unsigned long long";
}
inline std::string initializer_str(const unsigned long long& t) {
    return std::to_string(t) + "ull";
}
inline std::string repr(const unsigned long long& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const std::string&) {
    return "std::string";
}
inline std::string initializer_str(const std::string& t) {
    return "\"" + t + "\"";
}
inline std::string repr(const std::string& t) {
    return internal_repr::type_str(t) + "{" + internal_repr::initializer_str(t) + "}";
}

inline std::string type_str(const float&) {
    return "float";
}
inline std::string initializer_str(const float& t) {
    return std::to_string(t) + "f";
}
inline std::string repr(const float& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const double&) {
    return "double";
}
inline std::string initializer_str(const double& t) {
    return std::to_string(t);
}
inline std::string repr(const double& t) {
    return internal_repr::initializer_str(t);
}

inline std::string type_str(const long double&) {
    return "long double";
}
inline std::string initializer_str(const long double& t) {
    return std::to_string(t) + "l";
}
inline std::string repr(const long double& t) {
    return internal_repr::initializer_str(t);
}

template <typename T>
inline std::string type_str(const std::vector<T>&) {
    T t;
    return "std::vector<" + internal_repr::type_str(t) + ">";
}
template <typename T>
inline std::string initializer_str(const std::vector<T>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += internal_repr::initializer_str(*itr);
    }
    ret += "}";
    return ret;
}
template <typename T>
inline std::string repr(const std::vector<T>& t) {
    return internal_repr::type_str(t) + internal_repr::initializer_str(t);
}

template <typename T>
inline std::string type_str(const std::set<T>&) {
    T t;
    return "std::set<" + internal_repr::type_str(t) + ">";
}
template <typename T>
inline std::string initializer_str(const std::set<T>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += internal_repr::initializer_str(*itr);
    }
    ret += "}";
    return ret;
}
template <typename T>
inline std::string repr(const std::set<T>& t) {
    return internal_repr::type_str(t) + internal_repr::initializer_str(t);
}

template <typename T1, typename T2>
inline std::string type_str(const std::pair<T1, T2>&) {
    T1 t1;
    T2 t2;
    return "std::pair<" + internal_repr::type_str(t1) + ", " + internal_repr::type_str(t2) + ">";
}
template <typename T1, typename T2>
inline std::string initializer_str(const std::pair<T1, T2>& t) {
    return "{" + internal_repr::initializer_str(t.first) + ", " + internal_repr::initializer_str(t.second) + "}";
}
template <typename T1, typename T2>
inline std::string repr(const std::pair<T1, T2>& t) {
    return internal_repr::type_str(t) + internal_repr::initializer_str(t);
}

template <typename T1, typename T2>
inline std::string type_str(const std::map<T1, T2>&) {
    T1 t1;
    T2 t2;
    return "std::map<" + internal_repr::type_str(t1) + ", " + internal_repr::type_str(t2) + ">";
}
template <typename T1, typename T2>
inline std::string initializer_str(const std::map<T1, T2>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += "{";
        ret += internal_repr::initializer_str(itr->first);
        ret += ", ";
        ret += internal_repr::initializer_str(itr->second);
        ret += "}";
    }
    ret += "}";
    return ret;
}
template <typename T1, typename T2>
inline std::string repr(const std::map<T1, T2>& t) {
    return internal_repr::type_str(t) + internal_repr::initializer_str(t);
}

template <typename T, std::size_t N>
inline std::string type_str(const std::array<T, N>&) {
    T t;
    return "std::array<" + internal_repr::type_str(t) + ", " + std::to_string(N) + ">";
}
template <typename T, std::size_t N>
inline std::string initializer_str(const std::array<T, N>& t) {
    std::string ret;
    ret += "{";
    for (auto itr = t.begin(); itr != t.end(); itr++) {
        if (itr != t.begin()) ret += ", ";
        ret += internal_repr::initializer_str(*itr);
    }
    ret += "}";
    return ret;
}
template <typename T, std::size_t N>
inline std::string repr(const std::array<T, N>& t) {
    return internal_repr::type_str(t) + internal_repr::initializer_str(t);
}

}  // namespace internal_repr

template <typename T>
inline std::string type_str(const T& t) {
    return internal_repr::type_str(t);
}
template <typename T>
inline std::string initializer_str(const T& t) {
    return internal_repr::initializer_str(t);
}
template <typename T>
inline std::string repr(const T& t) {
    return internal_repr::repr(t);
}

#define DEBUG(x) \
    { std::cerr << #x << " = " << repr(x) << std::endl; }
