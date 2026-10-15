#pragma once

/**
 * @file counter.hpp
 * @brief 値の個数を数える多重集合 (Counter)
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/24638d870d2ac03d01e1498411ab8aa477ea366f/src/data_structures/counter.hpp

template <typename T>
struct Counter {
    using key_type = T;
    using mapped_type = long long;
    using value_type = std::pair<key_type, mapped_type>;
    std::map<key_type, mapped_type> data;

    void inc(const key_type& key, const mapped_type& t) {
        data[key] += t;
        auto itr = data.find(key);
        assert(itr->second >= 0);
        if (itr->second == 0) {
            data.erase(itr);
        }
    }
    Counter() = default;
    explicit Counter(const key_type& x) : data{{x, 1}} {}
    /**
     * @brief 範囲の要素を数える
     * Counter 自身と key_type (std::string など範囲でもあるもの) はこちらで受け取らない
     */
    template <class C, ENABLE_IF_T(!std::is_same_v<std::decay_t<C>, Counter> && !std::is_convertible_v<C, key_type>),
              class = decltype(std::begin(std::declval<C&>()), std::end(std::declval<C&>()))>
    explicit Counter(C&& v) {
        for (const key_type& a : v) {
            data[a]++;
        }
    }
    Counter(std::initializer_list<value_type> init) {
        for (const auto& p : init) {
            inc(p.first, p.second);
        }
    }
    Counter& operator+=(const Counter& o) {
        for (const auto& [i, c] : o.data) {
            data[i] += c;
        }
        return *this;
    }
    friend Counter operator+(const Counter& a, const Counter& b) {
        Counter res(a);
        return res += b;
    }
    Counter& operator-=(const Counter& o) {
        // 自分自身を引くと、走査中の o.data の要素を inc が消してしまう
        if (this == &o) {
            data.clear();
            return *this;
        }
        for (const auto& [i, c] : o.data) {
            inc(i, -c);
        }
        return *this;
    }
    friend Counter operator-(const Counter& a, const Counter& b) {
        Counter res(a);
        return res -= b;
    }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    /**
     * @brief 異なる値の個数 (重複は数えない)
     */
    long long size() const {
        return static_cast<long long>(data.size());
    }
    /**
     * @brief 重複を含めた要素数 (個数の総和)
     * @note Python の collections.Counter.total() と同じ。O(異なる値の個数)
     */
    mapped_type total() const {
        mapped_type ret = 0;
        for (const auto& [key, cnt] : data) ret += cnt;
        return ret;
    }
    mapped_type get(const key_type& key) const {
        auto itr = data.find(key);
        if (itr == data.end()) return 0;
        return itr->second;
    }
    void set(const key_type& key, const mapped_type& t) {
        assert(t >= 0);
        if (t > 0) {
            data[key] = t;
        } else {
            auto itr = data.find(key);
            if (itr != data.end()) {
                data.erase(itr);
            }
        }
    }
    std::vector<value_type> most_common() const {
        std::vector<value_type> ret;
        ret.reserve(data.size());
        for (const auto& [i, c] : data) {
            ret.push_back({i, c});
        }
        std::sort(ret.begin(), ret.end(), [](const value_type& a, const value_type& b) { return a.second > b.second || (a.second == b.second && a.first < b.first); });
        return ret;
    }
    class CounterReference {
        Counter& base;
        key_type key;

       public:
        CounterReference(Counter& base, key_type key) : base(base), key(key) {}
        CounterReference& operator=(const mapped_type& x) {
            base.set(key, x);
            return *this;
        }
        CounterReference& operator+=(const mapped_type& x) {
            base.inc(key, x);
            return *this;
        }
        CounterReference& operator-=(const mapped_type& x) {
            base.inc(key, -x);
            return *this;
        }
        operator mapped_type() const {
            return base.get(key);
        }
    };
    mapped_type operator[](const key_type& key) const {
        return get(key);
    }
    CounterReference operator[](const key_type& key) {
        return CounterReference(*this, key);
    }
};
