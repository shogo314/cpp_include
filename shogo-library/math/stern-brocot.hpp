#pragma once

/**
 * @file stern-brocot.hpp
 * @brief Stern–Brocot 木と連分数
 */

#include <algorithm>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "rational.hpp"
// https://github.com/shogo314/shogo-library/blob/93de89301cdeac655ce53761092d9556752fd185/src/math/stern-brocot.hpp

/**
 * @brief Stern–Brocot 木と連分数
 *
 * 正の有理数を Stern–Brocot 木 (根は 1/1) の頂点と同一視する
 * 根から頂点への道は、左の子へ進む 'L' と右の子へ進む 'R' を連長圧縮した {文字, 回数} の列で表す
 * 例:
 * stern_brocot::encode_path(Rational<>(3, 2));                     // {{'R', 1}, {'L', 1}}
 * stern_brocot::simplest_between(Rational<>(1, 3), Rational<>(1, 2));  // 2/5
 */
namespace stern_brocot {

/**
 * @brief 連分数展開 x = a0 + 1 / (a1 + 1 / (... + 1 / an))
 * a0 = floor(x)、i >= 1 では ai >= 1。n >= 1 なら an >= 2 (表し方が 1 通りに決まる)
 */
template <typename T>
std::vector<T> continued_fraction(const Rational<T>& x) {
    std::vector<T> res;
    T p = x.num(), q = x.den();
    T a = x.floor();
    res.push_back(a);
    p -= a * q;
    while (p != 0) {
        // 小数部分 p / q の逆数 q / p の整数部分
        T t = q / p, r = q - t * p;
        res.push_back(t);
        q = p, p = r;
    }
    return res;
}

/**
 * @brief 連分数 [a0; a1, ..., an] が表す有理数
 * @pre a は空でなく、i >= 1 では ai >= 1
 */
template <typename T>
Rational<T> from_continued_fraction(const std::vector<T>& a) {
    assert(!a.empty());
    // 後ろから p / q = ai + 1 / (p / q) をたどる
    T p = a.back(), q = 1;
    for (int i = static_cast<int>(a.size()) - 2; i >= 0; i--) {
        assert(a[i + 1] >= 1);
        T np = a[i] * p + q;
        q = p, p = np;
    }
    return Rational<T>(p, q);
}

/**
 * @brief 根 1/1 から x への道
 * @pre x > 0
 */
template <typename T>
std::vector<std::pair<char, T>> encode_path(const Rational<T>& x) {
    assert(x > 0);
    std::vector<T> a = continued_fraction(x);
    // x = [a0; a1, ..., an] への道は R^a0 L^a1 R^a2 ... で、最後の項だけ 1 少ない
    a.back() -= 1;
    std::vector<std::pair<char, T>> res;
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
        if (a[i] > 0) res.emplace_back(i % 2 == 0 ? 'R' : 'L', a[i]);
    }
    return res;
}

namespace internal {
// 道をたどったあとの頂点の、子孫の範囲の両端 (下端 lo、上端 hi)。分母が 0 なら +∞
template <typename T>
struct Bounds {
    T lo_num = 0, lo_den = 1, hi_num = 1, hi_den = 0;
    // 'R' へ n 回: 下端が今の頂点 (lo + hi) に n 回置き換わる
    void move(char c, T n) {
        assert(c == 'L' || c == 'R');
        if (c == 'R') {
            lo_num += n * hi_num, lo_den += n * hi_den;
        } else {
            hi_num += n * lo_num, hi_den += n * lo_den;
        }
    }
    Rational<T> node() const { return Rational<T>(lo_num + hi_num, lo_den + hi_den); }
};
template <typename T>
Bounds<T> walk(const std::vector<std::pair<char, T>>& path) {
    Bounds<T> b;
    for (auto [c, n] : path) {
        assert(n >= 0);
        b.move(c, n);
    }
    return b;
}
}  // namespace internal

/**
 * @brief 根 1/1 から道 path をたどった頂点
 * 同じ文字が続いてもよく、回数は 0 以上
 */
template <typename T>
Rational<T> decode_path(const std::vector<std::pair<char, T>>& path) {
    return internal::walk(path).node();
}

/**
 * @brief x の深さ (根 1/1 は 0)
 * @pre x > 0
 */
template <typename T>
T depth(const Rational<T>& x) {
    T d = 0;
    for (auto [c, n] : encode_path(x)) d += n;
    return d;
}

/**
 * @brief x と y の最小共通祖先
 * @pre x > 0、y > 0
 */
template <typename T>
Rational<T> lca(const Rational<T>& x, const Rational<T>& y) {
    std::vector<std::pair<char, T>> px = encode_path(x), py = encode_path(y), common;
    for (int i = 0; i < static_cast<int>(std::min(px.size(), py.size())); i++) {
        if (px[i].first != py[i].first) break;
        common.emplace_back(px[i].first, std::min(px[i].second, py[i].second));
        if (px[i].second != py[i].second) break;
    }
    return decode_path(common);
}

/**
 * @brief x の祖先で深さが k の頂点 (k > depth(x) なら std::nullopt)
 * @pre x > 0、k >= 0
 */
template <typename T>
std::optional<Rational<T>> ancestor(const Rational<T>& x, T k) {
    assert(k >= 0);
    std::vector<std::pair<char, T>> path;
    for (auto [c, n] : encode_path(x)) {
        if (k == 0) break;
        T m = std::min(n, k);
        path.emplace_back(c, m);
        k -= m;
    }
    if (k > 0) return std::nullopt;
    return decode_path(path);
}

/**
 * @brief x の子孫 (x 自身を含む) が覆う開区間の {{下端の分子, 分母}, {上端の分子, 分母}}
 * 下端は 0 なら {0, 1}、上端は +∞ なら {1, 0}
 * @pre x > 0
 */
template <typename T>
std::pair<std::pair<T, T>, std::pair<T, T>> range(const Rational<T>& x) {
    internal::Bounds<T> b = internal::walk(encode_path(x));
    return {{b.lo_num, b.lo_den}, {b.hi_num, b.hi_den}};
}

namespace internal {
// 0 <= a / b < c / d (b, d > 0) のとき、開区間 (a / b, c / d) で分母が最小の分数 {分子, 分母}
// 分母が最小のものは 1 つに決まり、分子も最小になる。掛け算をしないので a, b, c, d の範囲であふれない
template <typename T>
std::pair<T, T> simplest_open(T a, T b, T c, T d) {
    T k = a / b;
    // k + 1 < c / d なら k + 1
    if (k + 1 <= (c - 1) / d) return {k + 1, 1};
    // 整数部分 k を引く。0 <= a / b < 1、0 < c / d <= 1
    a -= k * b, c -= k * d;
    if (a == 0) {
        // 0 < 1 / q < c / d となる最小の q
        T q = d / c + 1;
        return {k * q + 1, q};
    }
    // 逆数をとると d / c < q / p < b / a
    auto [u, v] = simplest_open(d, c, b, a);
    return {k * u + v, u};
}
}  // namespace internal

/**
 * @brief lo と hi の間で最も単純な (分母が最小で、その中で分子の絶対値が最小の) 分数
 * 既定は開区間 (lo, hi)。include_lo, include_hi が true なら、その端も含める
 * 分母が最小のものは 1 つに決まる
 * @pre lo < hi (両端を含むなら lo <= hi)
 */
template <typename T>
Rational<T> simplest_between(const Rational<T>& lo, const Rational<T>& hi, bool include_lo = false, bool include_hi = false) {
    if (lo == hi) {
        assert(include_lo && include_hi);
        return lo;
    }
    assert(lo < hi);
    // 0 を含むなら 0
    if ((lo < 0 || (lo == 0 && include_lo)) && (0 < hi || (hi == 0 && include_hi))) return Rational<T>(0);
    // 負の区間は符号を反転して求める
    if (hi <= 0) return -simplest_between(-hi, -lo, include_hi, include_lo);
    auto [p, q] = internal::simplest_open(lo.num(), lo.den(), hi.num(), hi.den());
    Rational<T> res(p, q);
    // 含める端の方が単純なら、そちらを選ぶ
    auto simpler = [](const Rational<T>& x, const Rational<T>& y) {
        return x.den() != y.den() ? x.den() < y.den() : x.num() < y.num();
    };
    if (include_lo && simpler(lo, res)) res = lo;
    if (include_hi && simpler(hi, res)) res = hi;
    return res;
}

/**
 * @brief 分母が n 以下の分数のうち、x 以下で最大のものと x 以上で最小のもの {lower, upper}
 * x の分母が n 以下なら {x, x}
 * @pre n >= 1
 */
template <typename T>
std::pair<Rational<T>, Rational<T>> best_approximation(const Rational<T>& x, T n) {
    assert(n >= 1);
    if (x.den() <= n) return {x, x};
    // 整数部分を引いた 0 < x - k < 1 で木をたどる (分母は変わらない)
    T k = x.floor();
    internal::Bounds<T> b;
    for (auto [c, m] : encode_path(x - k)) {
        // 進むと動く方の端の分母が n を超えない回数だけ進む (足す方の端が +∞ なら分母は変わらない)
        T add_den = c == 'R' ? b.hi_den : b.lo_den, cur_den = c == 'R' ? b.lo_den : b.hi_den;
        T step = add_den == 0 ? m : (n - cur_den) / add_den;
        b.move(c, std::min(m, step));
        if (step < m) break;
    }
    return {Rational<T>(b.lo_num, b.lo_den) + k, Rational<T>(b.hi_num, b.hi_den) + k};
}

/**
 * @brief 分母が max_den 以下 (max_num を渡すと分子も max_num 以下) の 0 以上の分数の中で、単調な pred の境目を求める
 * pred(x) は小さい x で true、大きい x で false となる単調な関数で、pred(0) は true であること
 * 戻り値は {{pred が true の最大の分数の分子, 分母}, {pred が false の最小の分数の分子, 分母}}。false の側がなければ {1, 0} (+∞)
 * max_num を渡さないなら、pred はどこかで false になること (そうでないと終わらない)
 * pred を呼ぶ回数は O(log max_den) 程度 (max_num を渡さないなら O(log (max_den + 境目の値)))
 * @pre max_den >= 1
 */
template <typename T, typename Pred>
std::pair<std::pair<T, T>, std::pair<T, T>> binary_search(T max_den, Pred pred, std::optional<T> max_num = std::nullopt) {
    assert(max_den >= 1);
    assert(!max_num || *max_num >= 0);
    assert(pred(Rational<T>(0)));
    internal::Bounds<T> b;
    // 分子が 0 以下なら 0 しか選べない (上端 +∞ から左へ進んでも分子 1 のままなので、上限を満たせない)
    if (max_num && *max_num == 0) return {{0, 1}, {1, 0}};
    // 動く方の端 (num, den) に k 回もう一方の端 (add_num, add_den) を足せる回数の上限 (なければ -1)
    auto limit = [&](T num, T den, T add_num, T add_den) -> T {
        T res = -1;
        if (add_den > 0) res = (max_den - den) / add_den;
        if (max_num && add_num > 0) {
            T r = (*max_num - num) / add_num;
            res = res == -1 ? r : std::min(res, r);
        }
        return res;
    };
    // ok(k) が k = 0 で true、k が大きいと false になるとき、ok(k) が true の最大の k (k <= bound、bound = -1 なら上限なし)
    // 倍々に進めてから、通り過ぎた分を二分探索で戻す。ok を呼ぶ回数は O(log k)
    auto search = [](T bound, auto ok) -> T {
        T k = 0, s = 1;
        while ((bound == -1 || s <= bound - k) && ok(k + s)) k += s, s *= 2;
        while (s > 1) {
            s /= 2;
            if ((bound == -1 || s <= bound - k) && ok(k + s)) k += s;
        }
        return k;
    };
    while (true) {
        bool moved = false;
        // 下端を上げる: 下端 + k 上端 で pred が true の最大の k
        T k = search(limit(b.lo_num, b.lo_den, b.hi_num, b.hi_den), [&](T k) {
            return pred(Rational<T>(b.lo_num + k * b.hi_num, b.lo_den + k * b.hi_den));
        });
        if (k > 0) b.move('R', k), moved = true;
        // 上端を下げる: 上端 + k 下端 で pred が false の最大の k
        k = search(limit(b.hi_num, b.hi_den, b.lo_num, b.lo_den), [&](T k) {
            return !pred(Rational<T>(b.hi_num + k * b.lo_num, b.hi_den + k * b.lo_den));
        });
        if (k > 0) b.move('L', k), moved = true;
        if (!moved) break;
    }
    return {{b.lo_num, b.lo_den}, {b.hi_num, b.hi_den}};
}

}  // namespace stern_brocot
