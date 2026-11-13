#pragma once

/**
 * @file quotients.hpp
 * @brief 商の列挙 (floor(n / i)・floor((a x + b) / m) が一定の区間)
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>

#include "../base/functions.hpp"
// https://github.com/shogo314/shogo-library/blob/9a3850ecb2614059a50786efbc668c34530bbd74/src/math/quotients.hpp

/**
 * @brief 商 q と、それをとる添字の区間 [l, r)
 */
struct QuotientRange {
    long long q;  //!< 商
    long long l;  //!< 区間の左端 (含む)
    long long r;  //!< 区間の右端 (含まない)
};

namespace internal_quotients {
struct Quotients {
    long long n = 0;
    QuotientRange current{0, 1, 1};
    bool done = true;

    void advance() {
        const long long l = current.r;
        done = l > n;
        if (!done) {
            const long long q = n / l;
            current = {q, l, n / q + 1};
        }
    }
};

struct QuotientsLinear {
    long long n = 0, m = 1, a = 0, b = 0;
    QuotientRange current{0, 0, 0};
    bool done = true;

    void advance() {
        const long long l = current.r;
        done = l == n;
        if (done) return;
        if (a == 1) {
            long long q, length;
            if (l == 0) {
                auto [initial_q, rem] = divmod(static_cast<__int128_t>(b), static_cast<__int128_t>(m));
                q = static_cast<long long>(initial_q);
                length = m - static_cast<long long>(rem);
            } else {
                assert(current.q < std::numeric_limits<long long>::max());
                q = current.q + 1;
                length = m;
            }
            current = {q, l, l + std::min(length, n - l)};
            return;
        }
        advance_general(l);
    }

    // 加算だけで進む a == 1 の更新をインライン化しやすいよう、汎用計算を分ける。
    void advance_general(long long l) {
        const __int128_t slope = a;
        auto [q, rem] = divmod(slope * l + b, static_cast<__int128_t>(m));
        assert(std::numeric_limits<long long>::min() <= q && q <= std::numeric_limits<long long>::max());
        __int128_t length = n - l;
        // 正の傾きでは次の商に届くまで、負の傾きでは余りが負になるまで進む
        if (a > 0) chmin(length, ceildiv(m - rem, slope));
        if (a < 0) chmin(length, rem / (-slope) + 1);
        current = {static_cast<long long>(q), l, l + static_cast<long long>(length)};
    }
};

// イテレータが状態を所有するため、range の寿命や別の走査に依存しない。
template <class State>
class Enumeration {
    State initial;

   public:
    explicit Enumeration(State initial) : initial(initial) {}

    class iterator {
        State state;

       public:
        using value_type = QuotientRange;
        using difference_type = std::ptrdiff_t;
        using reference = value_type;
        using pointer = void;
        using iterator_category = std::input_iterator_tag;

        iterator() = default;
        explicit iterator(State state) : state(state) { this->state.advance(); }
        value_type operator*() const { return state.current; }
        iterator& operator++() {
            state.advance();
            return *this;
        }
        iterator operator++(int) {
            auto previous = *this;
            ++*this;
            return previous;
        }
        bool operator==(const iterator& other) const {
            if (state.done || other.state.done) return state.done == other.state.done;
            return state.current.l == other.state.current.l;
        }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    };

    iterator begin() const { return iterator(initial); }
    iterator end() const { return iterator(); }
};
}  // namespace internal_quotients

/**
 * @brief floor(n / i) が一定の区間を添字の昇順 (商の降順) に遅延列挙する。全体 O(sqrt(n))、追加領域 O(1)
 * @pre 0 <= n < std::numeric_limits<long long>::max()
 */
inline auto enumerate_quotients(long long n) {
    assert(0 <= n && n < std::numeric_limits<long long>::max());
    return internal_quotients::Enumeration(internal_quotients::Quotients{n});
}

/**
 * @brief x = 0, ..., n - 1 を floor((a x + b) / m) の値で区間に分け、x の昇順に遅延列挙する。全体 O(区間数)、追加領域 O(1)
 * @pre n >= 0、m >= 1、各商が long long に収まること。a, b は負でもよい
 */
inline auto enumerate_quotients_linear(long long n, long long m, long long a, long long b) {
    assert(n >= 0 && m >= 1);
    return internal_quotients::Enumeration(internal_quotients::QuotientsLinear{n, m, a, b});
}

/**
 * @brief floor(n / i) が一定の区間ごとに f(q, l, r) を呼ぶ O(sqrt(n))
 */
template <typename F>
void for_each_quotient(long long n, F f) {
    for (auto [q, l, r] : enumerate_quotients(n)) f(q, l, r);
}

/**
 * @brief floor((a x + b) / m) が一定の区間ごとに f(q, l, r) を呼ぶ O(区間数)
 */
template <typename F>
void for_each_quotient_linear(long long n, long long m, long long a, long long b, F f) {
    for (auto [q, l, r] : enumerate_quotients_linear(n, m, a, b)) f(q, l, r);
}
