#pragma once
#include <array>
#include <map>
#include <set>
#include <string_view>
#include <type_traits>
#include <vector>

template <typename T>
struct Counter : public std::map<T, long long> {
    using key_type = T;
    using mapped_type = long long;
    using std::map<key_type, mapped_type>::map;

    void add(const std::vector<key_type>& v) {
        for (const auto& i : v) {
            this->operator[](i)++;
        }
    }
    template <size_t N>
    void add(const std::array<key_type, N>& v) {
        for (const auto& i : v) {
            this->operator[](i)++;
        }
    }
    void add(const std::set<key_type>& v) {
        for (const auto& i : v) {
            this->operator[](i)++;
        }
    }
    template <std::enable_if_t<std::is_same_v<char, key_type>, std::nullptr_t> = nullptr>
    void add(const std::string_view& s) {
        for (const auto& c : s) {
            this->operator[](c)++;
        }
    }
    Counter(const key_type& x) : map<key_type, mapped_type>{} { this->operator[](x)++; }
    Counter(const std::vector<key_type>& v) : map<key_type, mapped_type>{} { add(v); }
    template <size_t N>
    Counter(const std::array<key_type, N>& v) { add(v); }
    Counter(const std::set<key_type>& v) { add(v); }
    template <std::enable_if_t<std::is_same_v<char, key_type>, std::nullptr_t> = nullptr>
    Counter(const std::string_view& s) { add(s); }

    bool contains(const key_type& x) const {
        return this->find(x) != this->end();
    }

    void inc(const key_type& x, const mapped_type& c = 1) {
        this->operator[](x) += c;
    }

    void dec(const key_type& x, const mapped_type& c = 1) {
        this->operator[](x) -= c;
        if (this->operator[](x) <= 0) {
            this->erase(x);
        }
    }

    Counter& operator+=(const Counter& o) {
        for (const auto [i, c] : o) {
            this->operator[](i) += c;
        }
        return *this;
    }
    friend Counter operator+(const Counter& a, const Counter& b) {
        Counter res(a);
        return res += b;
    }
};
