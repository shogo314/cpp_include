#pragma once

/**
 * @file counter.hpp
 * @brief 値の個数を数える多重集合 (Counter)
 */

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/8eadfabbff53ac04c3000e8f0585b08e4fe5155f/src/data_structures/counter.hpp

template <typename T = long long>
struct Counter {
    using key_type = T;
    using mapped_type = long long;
    using value_type = std::pair<key_type, mapped_type>;

   private:
    struct CompareFrequency {
        bool operator()(const value_type& a, const value_type& b) const {
            return a.second > b.second || (a.second == b.second && a.first < b.first);
        }
    };
    std::map<key_type, mapped_type> data;
    std::set<value_type, CompareFrequency> frequency;

   public:
    void inc(const key_type& key, const mapped_type& t) {
        set(key, get(key) + t);
    }
    Counter() = default;
    explicit Counter(const key_type& x) { set(x, 1); }
    /**
     * @brief 範囲の要素を数える
     * Counter 自身と key_type (std::string など範囲でもあるもの) はこちらで受け取らない
     */
    template <class C, ENABLE_IF_T(!std::is_same_v<std::decay_t<C>, Counter> && !std::is_convertible_v<C, key_type>),
              class = decltype(std::begin(std::declval<C&>()), std::end(std::declval<C&>()))>
    explicit Counter(C&& v) {
        for (const key_type& a : v) {
            inc(a, 1);
        }
    }
    Counter(std::initializer_list<value_type> init) {
        for (const auto& p : init) {
            inc(p.first, p.second);
        }
    }
    Counter& operator+=(const Counter& o) {
        for (const auto& [i, c] : o.data) {
            inc(i, c);
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
            frequency.clear();
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
    /** @brief key 以上の最初の値の読み取り専用イテレータ (なければ end()) */
    auto lower_bound(const key_type& key) const { return data.lower_bound(key); }
    /** @brief key より大きい最初の値の読み取り専用イテレータ (なければ end()) */
    auto upper_bound(const key_type& key) const { return data.upper_bound(key); }
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
        auto itr = data.find(key);
        if (itr != data.end()) frequency.erase(*itr);
        if (t > 0) {
            data[key] = t;
            frequency.emplace(key, t);
        } else if (itr != data.end()) {
            data.erase(itr);
        }
    }
    struct Mode {
        T value;
        long long count;
    };
    /**
     * @brief 最頻値とその個数を返す。同数なら最小の値を選び、空なら std::nullopt を返す
     * @note O(1)
     */
    std::optional<Mode> mode() const {
        if (frequency.empty()) return std::nullopt;
        const auto& [value, count] = *frequency.begin();
        return Mode{value, count};
    }
    /**
     * @brief 個数の降順、同数なら値の昇順に並べた内部 set の const 参照 (コピーしない)
     * @note O(1)。Counter の更新は参照先にも反映される。更新した値の要素へのイテレータは失効する
     */
    const auto& by_frequency() const { return frequency; }
    std::vector<value_type> most_common() const {
        return {frequency.begin(), frequency.end()};
    }
    /**
     * @brief 個数の多い順に上位 k 件を返す。同数なら値の昇順。k >= 0
     * @note k が異なる値の個数より大きければ全件を返す。O(min(k, size()))
     */
    std::vector<value_type> most_common(long long k) const {
        assert(k >= 0);
        if (k > size()) k = size();
        std::vector<value_type> result;
        result.reserve(k);
        auto itr = frequency.begin();
        for (long long i = 0; i < k; ++i, ++itr) result.push_back(*itr);
        return result;
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

template <typename T>
Counter(std::vector<T>) -> Counter<T>;
