#pragma once
#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <type_traits>
#include <vector>
/**
 * @brief 半開区間を管理する
 * @tparam T スカラー
 */
template <typename T = long long, class Compare = std::less<T>, std::enable_if_t<std::is_scalar_v<T>>* = nullptr>
struct IntervalSet {
   public:
    using key_type = T;
    using value_type = std::pair<key_type, key_type>;
    inline constexpr static auto key_comp = Compare();
    struct value_compare {
        bool operator()(const value_type& a, const value_type& b) const {
            return key_comp(a.first, b.first) or (not key_comp(b.first, a.first) and key_comp(b.second, a.second));
        }
    };
    inline constexpr static auto value_comp = value_compare();

   private:
    std::set<value_type, value_compare> data;
    using iterator = typename decltype(data)::iterator;
    std::optional<std::pair<iterator, iterator>> find(key_type l, key_type r) const {
        auto itr_l = data.upper_bound({l, l});
        if (itr_l == data.begin()) {
        } else {
            if (not key_comp(std::prev(itr_l)->second, l)) {
                --itr_l;
            }
        }
        if (itr_l == data.end() or key_comp(r, itr_l->first)) {
            return {};
        }
        if (key_comp(itr_l->first, l)) l = itr_l->first;
        auto itr_r = data.upper_bound({r, r});
        return std::pair<iterator, iterator>{itr_l, itr_r};
    }

   public:
    IntervalSet() = default;
    /**
     * @brief k が含まれている区間
     */
    std::optional<value_type> covered_by(key_type k) const {
        auto itr = data.upper_bound({k, k});
        if (itr == data.begin()) return {};
        --itr;
        if (key_comp(k, itr->second)) {
            return *itr;
        }
        return {};
    }
    /**
     * @brief [l,r) が完全に含まれている区間
     */
    std::optional<value_type> covered_by(key_type l, key_type r) const {
        auto opt = find(l, r);
        if (not opt.has_value()) {
            return {};
        }
        std::pair<iterator, iterator> p = opt.value();
        if (p.first != std::prev(p.second)) {
            return {};
        }
        if (key_comp(l, p.first->first)) return {};
        if (key_comp(p.first->second, r)) return {};
        return value_type(p.first->first, p.first->second);
    }
    void insert(key_type l, key_type r) {
        assert(key_comp(l, r));
        auto opt = find(l, r);
        if (not opt.has_value()) {
            data.emplace(l, r);
            return;
        }
        auto [itr_l, itr_r] = opt.value();
        if (key_comp(itr_l->first, l)) l = itr_l->first;
        if (key_comp(r, std::prev(itr_r)->second)) r = std::prev(itr_r)->second;
        data.erase(itr_l, itr_r);
        data.emplace(l, r);
    };
    void erase(key_type l, key_type r) {
        assert(key_comp(l, r));
        auto opt = find(l, r);
        if (not opt.has_value()) {
            return;
        }
        auto [itr_l, itr_r] = opt.value();
        std::vector<value_type> tmp;
        if (key_comp(itr_l->first, l)) tmp.push_back({itr_l->first, l});
        if (key_comp(r, std::prev(itr_r)->second)) tmp.push_back({r, std::prev(itr_r)->second});
        data.erase(itr_l, itr_r);
        while (not tmp.empty()) {
            data.insert(tmp.back());
            tmp.pop_back();
        }
    }
    std::size_t size() const {
        return data.size();
    }
    std::vector<value_type> to_vector() const {
        return std::vector<value_type>(data.begin(), data.end());
    }
};
