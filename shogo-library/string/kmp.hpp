#pragma once
// https://github.com/shogo314/shogo-library/blob/9a3850ecb2614059a50786efbc668c34530bbd74/src/string/kmp.hpp

/**
 * @file kmp.hpp
 * @brief 接頭辞関数と KMP 法
 */

#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

/**
 * @brief 接頭辞関数 (KMP 法の失敗関数)
 * s[0, i] の接頭辞であり接尾辞でもある部分 (s[0, i] 自身を除く) の最長の長さを各 i について求める 全体で O(N)
 *
 * @tparam It 配列のイテレータ
 * @param begin 配列の開始イテレータ
 * @param end 配列の終了イテレータ
 * @return std::vector<long long> pi
 */
template <typename It>
std::vector<long long> prefix_function(It begin, It end) {
    long long n = end - begin;
    std::vector<long long> pi(n);
    for (long long i = 1; i < n; i++) {
        long long j = pi[i - 1];
        while (j > 0 && !(begin[i] == begin[j])) j = pi[j - 1];
        if (begin[i] == begin[j]) j++;
        pi[i] = j;
    }
    return pi;
}

/**
 * @brief 接頭辞関数 (KMP 法の失敗関数)
 * s[0, i] の接頭辞であり接尾辞でもある部分 (s[0, i] 自身を除く) の最長の長さを各 i について求める 全体で O(N)
 *
 * @tparam R 配列の型
 * @param s 配列
 * @return std::vector<long long> pi
 */
template <typename R>
std::vector<long long> prefix_function(const R& s) {
    return prefix_function(std::begin(s), std::end(s));
}

namespace internal_kmp {
template <typename Text, typename Pattern>
class Matches {
    Text text;
    Pattern pattern;
    std::vector<long long> pi;
    long long n, m;

   public:
    Matches(Text&& text, Pattern&& pattern)
        : text(std::forward<Text>(text)), pattern(std::forward<Pattern>(pattern)), pi(prefix_function(this->pattern)), n(std::end(std::as_const(this->text)) - std::begin(std::as_const(this->text))), m(pi.size()) {}

    class iterator {
        const Matches* range = nullptr;
        long long i = 0, j = 0, pos = -1;

       public:
        using value_type = long long;
        using difference_type = std::ptrdiff_t;
        using reference = value_type;
        using pointer = void;
        using iterator_category = std::input_iterator_tag;

        iterator() = default;
        explicit iterator(const Matches* range) : range(range) { ++*this; }
        value_type operator*() const { return pos; }
        iterator& operator++() {
            if (range->m == 0) {
                pos = i;
                i = i == range->n ? -1 : i + 1;
                return *this;
            }
            auto t = std::begin(std::as_const(range->text));
            auto p = std::begin(std::as_const(range->pattern));
            while (i < range->n) {
                while (j > 0 && !(t[i] == p[j])) j = range->pi[j - 1];
                if (t[i] == p[j]) j++;
                i++;
                if (j == range->m) {
                    pos = i - range->m;
                    j = range->pi[j - 1];
                    return *this;
                }
            }
            pos = -1;
            return *this;
        }
        iterator operator++(int) {
            auto previous = *this;
            ++*this;
            return previous;
        }
        bool operator==(const iterator& other) const {
            if (pos == -1 || other.pos == -1) return pos == other.pos;
            return range == other.range && pos == other.pos;
        }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    };

    iterator begin() const { return iterator(this); }
    iterator end() const { return iterator(); }
};
}  // namespace internal_kmp

/**
 * @brief KMP 法による一致位置の遅延列挙。構築 O(|pattern|)、全走査 O(|text| + |pattern|)、追加領域 O(|pattern|)
 * pattern が空なら 0, 1, ..., |text| を返す。左辺値の入力は参照し、右辺値は所有する
 * @pre 両方の入力にランダムアクセス可能な iterator があること
 * @pre 走査中は range と参照する入力を有効なまま保持し、変更しないこと
 */
template <typename R1, typename R2>
auto kmp_search(R1&& text, R2&& pattern) {
    return internal_kmp::Matches<R1, R2>(std::forward<R1>(text), std::forward<R2>(pattern));
}
