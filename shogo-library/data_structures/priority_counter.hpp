#pragma once

/**
 * @file priority_counter.hpp
 * @brief 上位 K 個の値とその個数
 */

#include <array>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include "../utility/traits.hpp"
// https://github.com/shogo314/shogo-library/blob/93de89301cdeac655ce53761092d9556752fd185/src/data_structures/priority_counter.hpp

template <typename T, std::size_t K = 1, class Compare = std::less<T>, ENABLE_IF_T(K >= 1)>
struct PriorityCounter {
    using key_type = T;
    using mapped_type = long long;
    using value_type = std::pair<key_type, mapped_type>;
    inline constexpr static std::size_t k = K;

    // C++17 の constexpr コンストラクタではすべてのメンバの初期化が必要なので、値初期化しておく
    std::array<value_type, K> data{};

    constexpr PriorityCounter() {}
    explicit constexpr PriorityCounter(const key_type& x) : PriorityCounter() {
        data[0].first = x;
        data[0].second = 1;
    }

    constexpr long long size() const {
        for (std::size_t i = 0; i < k; i++) {
            if (data[i].second == 0) {
                return static_cast<long long>(i);
            }
        }
        return k;
    }

    constexpr void inc(const key_type& x, const mapped_type& c = 1) {
        bool f = false;
        key_type pre_key{};
        mapped_type pre_mapped{};
        for (std::size_t i = 0; i < k; i++) {
            if (f) {
                // std::swap は C++17 では constexpr でないので手で入れ替える
                key_type tmp_key = data[i].first;
                data[i].first = pre_key;
                pre_key = tmp_key;
                mapped_type tmp_mapped = data[i].second;
                data[i].second = pre_mapped;
                pre_mapped = tmp_mapped;
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
            } else if (!Compare()(x, data[i].first)) {
                data[i].second += c;
                return;
            }
        }
    }

    constexpr PriorityCounter& operator+=(const PriorityCounter& o) {
        for (std::size_t i = 0; i < k; i++) {
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

template <typename T, std::size_t K = 1>
using MaxCounter = PriorityCounter<T, K>;

template <typename T, std::size_t K = 1>
using MinCounter = PriorityCounter<T, K, std::greater<T>>;
