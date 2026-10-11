#pragma once
#include <array>
#include <cstddef>
#include <functional>
#include <map>
#include <type_traits>
#include <utility>

template <typename T, size_t K = 1, class Compare = std::less<T>, std::enable_if_t<(K >= 1), std::nullptr_t> = nullptr>
struct PriorityCounter {
    using key_type = T;
    using mapped_type = long long;
    using value_type = std::pair<key_type, mapped_type>;
    inline constexpr static size_t k = K;

    std::array<value_type, K> data;

    constexpr PriorityCounter() {
        for (size_t i = 0; i < k; i++) {
            data[i].second = 0;
        }
    }
    constexpr PriorityCounter(const key_type& x) : PriorityCounter() {
        data[0].first = x;
        data[0].second = 1;
    }

    constexpr size_t size() const {
        for (size_t i = 0; i < k; i++) {
            if (data[i].second == 0) {
                return i;
            }
        }
        return k;
    }

    constexpr void inc(const key_type& x, const mapped_type& c = 1) {
        bool f = false;
        key_type pre_key;
        mapped_type pre_mapped;
        for (size_t i = 0; i < k; i++) {
            if (f) {
                std::swap(pre_key, data[i].first);
                std::swap(pre_mapped, data[i].second);
                continue;
            }
            if (data[i].second == 0) {
                data[i].first = x;
                data[i].second += c;
                return;
            }
            if (Compare()(data[i].first, x)) {
                pre_key = data[i].first;
                pre_mapped = data[i].second;
                data[i].first = x;
                data[i].second = c;
                f = true;
            } else if (not Compare()(x, data[i].first)) {
                data[i].second += c;
                return;
            }
        }
    }

    constexpr PriorityCounter& operator+=(const PriorityCounter& o) {
        for (size_t i = 0; i < k; i++) {
            if (o.data[i].second == 0) break;
            this->inc(o.data[i].first, o.data[i].second);
        }
        return *this;
    }
    constexpr friend PriorityCounter operator+(const PriorityCounter& a, const PriorityCounter& b) {
        PriorityCounter res(a);
        return res += b;
    }
};

template <typename T, size_t K = 1>
using MaxCounter = PriorityCounter<T, K>;

template <typename T, size_t K = 1>
using MinCounter = PriorityCounter<T, K, std::greater<T>>;
