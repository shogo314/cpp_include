#pragma once
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/base/io.hpp
/**
 * @file io.hpp
 * @brief 空白区切り出力、iostreamのオーバーロード
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace internal_io {
// 128 bit の割り算は遅いので、18 桁ずつ 64 bit で扱う
constexpr unsigned long long POW10_18 = 1000000000000000000ULL;
// 空白で区切られた 1 語を 10 進の整数として読む。数でない、または lim を超えるなら false
template <typename CharT, typename Traits>
bool read_u128(std::basic_istream<CharT, Traits>& is, unsigned __int128& x, bool& neg, unsigned __int128 pos_max, unsigned __int128 neg_max) {
    std::basic_string<CharT, Traits> s;
    if (!(is >> s)) return false;
    std::size_t i = 0;
    neg = false;
    if (s[0] == CharT('-') || s[0] == CharT('+')) neg = s[i++] == CharT('-');
    if (i == s.size()) return false;
    for (std::size_t j = i; j < s.size(); j++) {
        if (s[j] < CharT('0') || CharT('9') < s[j]) return false;
    }
    while (i + 1 < s.size() && s[i] == CharT('0')) i++;
    // 39 桁なら 2^128 を超えることがあるので、最後の 1 桁の前で比べる。40 桁以上は必ず範囲外
    const std::size_t len = s.size() - i;
    if (len > 39) return false;
    const unsigned __int128 lim = neg ? neg_max : pos_max;
    const std::size_t safe = len == 39 ? s.size() - 1 : s.size();
    x = 0;
    while (i < safe) {
        const std::size_t k = std::min<std::size_t>(18, safe - i);
        unsigned long long c = 0, p = 1;
        for (std::size_t j = 0; j < k; j++, i++) c = c * 10 + (s[i] - CharT('0')), p *= 10;
        x = x * p + c;
    }
    if (len == 39) {
        const unsigned d = s[i] - CharT('0');
        if (x > lim / 10 || x * 10 > lim - d) return false;
        x = x * 10 + d;
    }
    return x <= lim;
}
// x < 10^18 を書く。pad なら 18 桁になるよう 0 で埋める。末尾から書いて先頭を返す
inline char* write_u64(char* end, unsigned long long x, bool pad) {
    char* p = end;
    do {
        *--p = static_cast<char>('0' + x % 10);
        x /= 10;
    } while (x > 0);
    if (pad) {
        while (p > end - 18) *--p = '0';
    }
    return p;
}
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& write_u128(std::basic_ostream<CharT, Traits>& os, unsigned __int128 x, bool neg) {
    char buf[41];
    char* p = buf + 40;
    *p = '\0';
    if (x < POW10_18) {
        p = write_u64(p, static_cast<unsigned long long>(x), false);
    } else {
        const unsigned __int128 q = x / POW10_18;
        p = write_u64(p, static_cast<unsigned long long>(x - q * POW10_18), true);
        if (q < POW10_18) {
            p = write_u64(p, static_cast<unsigned long long>(q), false);
        } else {
            const unsigned long long hi = static_cast<unsigned long long>(q / POW10_18);
            p = write_u64(p, static_cast<unsigned long long>(q - static_cast<unsigned __int128>(hi) * POW10_18), true);
            p = write_u64(p, hi, false);
        }
    }
    if (neg) *--p = '-';
    return os << p;
}
}  // namespace internal_io

// 128 bit 整数の入出力 (GNU 拡張)。std::basic_istream / std::basic_ostream には用意されていない
template <typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, __int128_t& x) {
    const unsigned __int128 pos_max = static_cast<unsigned __int128>(-1) >> 1;
    unsigned __int128 u;
    bool neg;
    if (internal_io::read_u128(is, u, neg, pos_max, pos_max + 1)) {
        x = neg ? static_cast<__int128_t>(-u) : static_cast<__int128_t>(u);
    } else {
        x = 0;
        is.setstate(std::ios_base::failbit);
    }
    return is;
}
template <typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, __uint128_t& x) {
    unsigned __int128 u;
    bool neg;
    if (internal_io::read_u128(is, u, neg, static_cast<unsigned __int128>(-1), 0)) {
        x = u;
    } else {
        x = 0;
        is.setstate(std::ios_base::failbit);
    }
    return is;
}
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, __int128_t x) {
    // -x は最小値であふれるので、符号なしで符号を反転する
    return internal_io::write_u128(os, x < 0 ? -static_cast<unsigned __int128>(x) : static_cast<unsigned __int128>(x), x < 0);
}
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, __uint128_t x) {
    return internal_io::write_u128(os, x, false);
}

// ネストした型 (pair<vector<int>, int> など) を扱えるように先に宣言しておく
template <typename T1, typename T2, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::pair<T1, T2>& p);
template <typename... Types, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::tuple<Types...>& p);
template <typename T, std::size_t N, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::array<T, N>& a);
template <typename T, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::vector<T>& v);
template <typename T1, typename T2, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::pair<T1, T2>& p);
template <typename... Types, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::tuple<Types...>& p);
template <typename T, std::size_t N, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::array<T, N>& a);
template <typename T, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::vector<T>& v);
template <typename T, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::set<T>& s);

namespace internal_io {
template <typename Tuple, std::size_t I, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& read_tuple(std::basic_istream<CharT, Traits>& is, Tuple& t) {
    is >> std::get<I>(t);
    if constexpr (I + 1 < std::tuple_size_v<Tuple>) {
        return read_tuple<Tuple, I + 1>(is, t);
    }
    return is;
}
template <typename Tuple, std::size_t I, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& write_tuple(std::basic_ostream<CharT, Traits>& os, const Tuple& t) {
    os << std::get<I>(t);
    if constexpr (I + 1 < std::tuple_size_v<Tuple>) {
        os << CharT(' ');
        return write_tuple<Tuple, I + 1>(os, t);
    }
    return os;
}
};  // namespace internal_io

template <typename T1, typename T2, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::pair<T1, T2>& p) {
    is >> p.first >> p.second;
    return is;
}
template <typename... Types, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::tuple<Types...>& p) {
    return internal_io::read_tuple<std::tuple<Types...>, 0>(is, p);
}
template <typename T, std::size_t N, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::array<T, N>& a) {
    for (auto& e : a) is >> e;
    return is;
}
template <typename T, typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, std::vector<T>& v) {
    for (auto& e : v) is >> e;
    return is;
}

template <typename T1, typename T2, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::pair<T1, T2>& p) {
    os << p.first << CharT(' ') << p.second;
    return os;
}
template <typename... Types, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::tuple<Types...>& p) {
    return internal_io::write_tuple<std::tuple<Types...>, 0>(os, p);
}
template <typename T, std::size_t N, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::array<T, N>& a) {
    for (std::size_t i = 0; i < N; ++i) {
        if (i) os << CharT(' ');
        os << a[i];
    }
    return os;
}
template <typename T, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::vector<T>& v) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) os << CharT(' ');
        os << v[i];
    }
    return os;
}
template <typename T, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::set<T>& s) {
    for (auto itr = s.begin(); itr != s.end(); ++itr) {
        if (itr != s.begin()) os << CharT(' ');
        os << *itr;
    }
    return os;
}

/**
 * @brief 空行出力
 */
inline void print() { std::cout << '\n'; }
/**
 * @brief 出力して改行
 *
 * @tparam T 型
 * @param x 出力する値
 */
template <typename T>
void print(const T& x) { std::cout << x << '\n'; }
/**
 * @brief 空白区切りで出力して改行
 *
 * @tparam T 1つ目の要素の型
 * @tparam Tail 2つ目以降の要素の型
 * @param x 1つ目の要素
 * @param tail 2つ目以降の要素
 */
template <typename T, typename... Tail>
void print(const T& x, const Tail&... tail) {
    std::cout << x << ' ';
    print(tail...);
}

/**
 * @brief 標準エラー出力に空行出力
 */
inline void err() { std::cerr << std::endl; }
/**
 * @brief 標準エラー出力に出力して改行
 *
 * @tparam T 型
 * @param x 出力する値
 */
template <typename T>
void err(const T& x) { std::cerr << x << std::endl; }
/**
 * @brief 標準エラー出力に空白区切りで出力して改行
 *
 * @tparam T 1つ目の要素の型
 * @tparam Tail 2つ目以降の要素の型
 * @param x 1つ目の要素
 * @param tail 2つ目以降の要素
 */
template <typename T, typename... Tail>
void err(const T& x, const Tail&... tail) {
    std::cerr << x << ' ';
    err(tail...);
}
