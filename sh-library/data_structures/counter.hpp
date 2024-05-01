#pragma once
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <map>
#include <vector>

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
    explicit Counter(const key_type& x) : data{{x, 1}} {}
    template <class C>
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
        for (const auto [i, c] : o) {
            data[i] += c;
        }
        return *this;
    }
    friend Counter operator+(const Counter& a, const Counter& b) {
        Counter res(a);
        return res += b;
    }
    Counter& operator-=(const Counter& o) {
        for (const auto [i, c] : o) {
            inc(i, -c);
        }
        return *this;
    }
    friend Counter operator-(const Counter& a, const Counter& b) {
        Counter res(a);
        return res -= b;
    }
    std::size_t size() const {
        return data.size();
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
        std::sort(ret.begin(), ret.end(), [](const value_type& a, const value_type& b) { return a.second > b.second or (a.second == b.second and a.first < b.first); });
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
