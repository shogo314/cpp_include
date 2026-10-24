#pragma once

/**
 * @file fps.hpp
 * @brief 形式的冪級数
 */

#include <algorithm>
#include <cassert>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

#include "combinatorics.hpp"
#include "convolution.hpp"
#include "modular-arithmetic.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/fps.hpp

template <typename mint>
struct FPS;

namespace internal_fps {
template <typename mint>
FPS<mint> gcd(const FPS<mint>& f, const FPS<mint>& g);

// 以下は法が NTT に向く素数のときの inv・exp。NTT した値を使い回し、巡回畳み込みで汚れない係数だけを取り出す。
// butterfly の結果はビット反転した順に並ぶので、長さ 2m の変換の前半は、x^m - 1 で割った余りの長さ m の変換になる

// 長さ 2^k の NTT と逆変換 (n で割る)
template <typename mint>
void ntt(std::vector<mint>& a) {
    internal_convolution::butterfly(a);
}
template <typename mint>
void intt(std::vector<mint>& a) {
    internal_convolution::butterfly_inv(a);
    const mint in = mint(a.size()).inv();
    for (auto& x : a) x *= in;
}

// 長さ n 以上の変換を mint で直接 NTT できるか (internal_convolution::ntt_candidate<mint>() のときだけ呼ぶ)
template <typename mint>
bool use_ntt(long long n) {
    long long z = 1;
    while (z < n) z <<= 1;
    return z <= (1 << 23) && internal_convolution::ntt_friendly<mint>(static_cast<int>(z));
}

// 1 / f の先頭 deg 項 (f[0] != 0、deg >= 1)。1 段 (m -> 2m) あたり長さ 2m の NTT 5 回
template <typename mint>
std::vector<mint> inv_ntt(const std::vector<mint>& f, long long deg) {
    std::vector<mint> g = {f[0].inv()};
    for (long long m = 1; m < deg; m <<= 1) {
        // F G = 1 + (x^m 以上の項)。巡回畳み込みの折り返しは [0, m) にしか来ない
        std::vector<mint> F(2 * m), G(2 * m);
        std::copy(f.begin(), f.begin() + std::min<long long>(f.size(), 2 * m), F.begin());
        std::copy(g.begin(), g.end(), G.begin());
        ntt(F), ntt(G);
        for (long long i = 0; i < 2 * m; i++) F[i] *= G[i];
        intt(F);
        std::fill(F.begin(), F.begin() + m, mint(0));
        // g <- g - g (F G - 1) の [m, 2m) の部分
        ntt(F);
        for (long long i = 0; i < 2 * m; i++) F[i] *= G[i];
        intt(F);
        for (long long i = m; i < 2 * m; i++) g.push_back(-F[i]);
    }
    g.resize(deg);
    return g;
}

// exp(f) の先頭 deg 項 (f[0] == 0、deg >= 1)。exp(f) = b と 1 / b を同時にニュートン法で倍々に伸ばす
template <typename mint>
std::vector<mint> exp_ntt(const std::vector<mint>& f, long long deg) {
    using C = Combination<mint>;
    C::extend(2 * deg);
    auto at = [&](long long i) { return i < static_cast<long long>(f.size()) ? f[i] : mint(0); };
    // b = exp(f) mod x^m、c = 1 / b mod x^{m/2}、Zc = c の長さ m の変換
    std::vector<mint> b = {1, at(1)}, c = {1}, Zc = {1, 1};
    for (long long m = 2; m < deg; m <<= 1) {
        std::vector<mint> Y(b);
        Y.resize(2 * m);
        ntt(Y);
        // c を 1 / b mod x^m に伸ばす: c <- c - c (b c - 1)。b c - 1 は [m/2, m) だけ取り出せば折り返しに汚れない
        {
            std::vector<mint> z(m);
            for (long long i = 0; i < m; i++) z[i] = Y[i] * Zc[i];
            intt(z);
            std::fill(z.begin(), z.begin() + m / 2, mint(0));
            ntt(z);
            for (long long i = 0; i < m; i++) z[i] *= -Zc[i];
            intt(z);
            c.insert(c.end(), z.begin() + m / 2, z.end());
        }
        Zc = c;
        Zc.resize(2 * m);
        ntt(Zc);
        // R = b f' - b' (f' は x^{m-1} 未満)。x^{m-1} 未満は 0 で、巡回畳み込みで [0, m-1) に折り返った x^m 以上の項を戻す
        std::vector<mint> x(m);
        for (long long i = 0; i < m - 1; i++) x[i] = at(i + 1) * (i + 1);
        ntt(x);
        for (long long i = 0; i < m; i++) x[i] *= Y[i];
        intt(x);
        for (long long i = 0; i < m - 1; i++) x[i] -= b[i + 1] * (i + 1);
        x.resize(2 * m);
        for (long long i = 0; i < m - 1; i++) x[m + i] = x[i], x[i] = 0;
        // R / b = f' - b' / b の [m - 1, 2m - 1) を積分すると、f - log b の [m, 2m)
        ntt(x);
        for (long long i = 0; i < 2 * m; i++) x[i] *= Zc[i];
        intt(x);
        for (long long i = 2 * m - 1; i >= m; i--) x[i] = x[i - 1] * C::inv(i) + at(i);
        std::fill(x.begin(), x.begin() + m, mint(0));
        // b <- b + b (f - log b) の [m, 2m)
        ntt(x);
        for (long long i = 0; i < 2 * m; i++) x[i] *= Y[i];
        intt(x);
        b.insert(b.end(), x.begin() + m, x.end());
    }
    b.resize(deg);
    return b;
}
}  // namespace internal_fps

template <typename mint>
struct FPS : std::vector<mint> {
    using std::vector<mint>::vector;

    // constructor
    FPS(const std::vector<mint>& r) : std::vector<mint>(r) {}

    // core operator
    inline FPS pre(long long siz) const {
        return FPS(begin(*this), begin(*this) + std::min(static_cast<long long>(this->size()), siz));
    }
    inline FPS rev() const {
        FPS res = *this;
        reverse(begin(res), end(res));
        return res;
    }
    inline FPS& normalize() {
        while (!this->empty() && this->back() == 0) this->pop_back();
        return *this;
    }

    // basic operator
    inline FPS operator-() const noexcept {
        FPS res = (*this);
        for (int i = 0; i < static_cast<int>(res.size()); ++i) res[i] = -res[i];
        return res;
    }
    inline FPS operator+(const mint& v) const { return FPS(*this) += v; }
    inline FPS operator+(const FPS& r) const { return FPS(*this) += r; }
    inline FPS operator-(const mint& v) const { return FPS(*this) -= v; }
    inline FPS operator-(const FPS& r) const { return FPS(*this) -= r; }
    inline FPS operator*(const mint& v) const { return FPS(*this) *= v; }
    inline FPS operator*(const FPS& r) const { return FPS(*this) *= r; }
    inline FPS operator/(const mint& v) const { return FPS(*this) /= v; }
    inline FPS operator<<(long long x) const { return FPS(*this) <<= x; }
    inline FPS operator>>(long long x) const { return FPS(*this) >>= x; }
    inline FPS& operator+=(const mint& v) {
        if (this->empty()) this->resize(1);
        (*this)[0] += v;
        return *this;
    }
    inline FPS& operator+=(const FPS& r) {
        if (r.size() > this->size()) this->resize(r.size());
        for (int i = 0; i < static_cast<int>(r.size()); ++i) (*this)[i] += r[i];
        return this->normalize();
    }
    inline FPS& operator-=(const mint& v) {
        if (this->empty()) this->resize(1);
        (*this)[0] -= v;
        return *this;
    }
    inline FPS& operator-=(const FPS& r) {
        if (r.size() > this->size()) this->resize(r.size());
        for (int i = 0; i < static_cast<int>(r.size()); ++i) (*this)[i] -= r[i];
        return this->normalize();
    }
    inline FPS& operator*=(const mint& v) {
        for (int i = 0; i < static_cast<int>(this->size()); ++i) (*this)[i] *= v;
        return *this;
    }
    inline FPS& operator*=(const FPS& r) {
        return *this = convolution((*this), r);
    }
    inline FPS& operator/=(const mint& v) {
        assert(v != 0);
        mint iv = v.inv();
        for (int i = 0; i < static_cast<int>(this->size()); ++i) (*this)[i] *= iv;
        return *this;
    }
    inline FPS& operator<<=(long long x) {
        FPS res(x, 0);
        res.insert(res.end(), begin(*this), end(*this));
        return *this = res;
    }
    inline FPS& operator>>=(long long x) {
        if (static_cast<long long>(this->size()) <= x) return *this = FPS<mint>(1, 0);
        FPS res;
        res.insert(res.end(), begin(*this) + x, end(*this));
        return *this = res;
    }
    inline mint eval(const mint& v) {
        mint res = 0;
        for (int i = static_cast<int>(this->size()) - 1; i >= 0; --i) {
            res *= v;
            res += (*this)[i];
        }
        return res;
    }
    // 最高次の係数が 1 の gcd (Half-GCD)。f と g がともに 0 なら 0 (空)
    inline friend FPS gcd(const FPS& f, const FPS& g) {
        return internal_fps::gcd(f, g);
    }

    // advanced operation
    // df/dx
    inline friend FPS diff(const FPS& f) {
        int n = static_cast<int>(f.size());
        if (n == 0) return FPS();
        FPS res(n - 1);
        for (int i = 1; i < n; ++i) res[i - 1] = f[i] * i;
        return res;
    }

    // \int f dx
    inline friend FPS intg(const FPS& f) {
        int n = static_cast<int>(f.size());
        FPS res(n + 1, 0);
        for (int i = 0; i < n; ++i) res[i + 1] = f[i] / (i + 1);
        return res;
    }

    // inv(f), f[0] must not be 0
    inline friend FPS inv(const FPS& f, long long deg) {
        assert(!f.empty() && f[0] != 0);
        if (deg < 0) deg = static_cast<long long>(f.size());
        if (deg == 0) return FPS();
        if constexpr (internal_convolution::ntt_candidate<mint>()) {
            if (internal_fps::use_ntt<mint>(2 * deg)) return internal_fps::inv_ntt<mint>(f, deg);
        }
        FPS res({mint(1) / f[0]});
        for (long long i = 1; i < deg; i <<= 1) {
            res = (res + res - res * res * f.pre(i << 1)).pre(i << 1);
        }
        res.resize(deg);
        return res;
    }
    inline friend FPS inv(const FPS& f) {
        return inv(f, f.size());
    }

    // division, r must be normalized (r.back() must not be 0)
    inline FPS& operator/=(const FPS& r) {
        assert(!r.empty());
        assert(r.back() != 0);
        this->normalize();
        if (this->size() < r.size()) {
            this->clear();
            return *this;
        }
        const long long need = static_cast<long long>(this->size()) - static_cast<long long>(r.size()) + 1;
        *this = ((*this).rev().pre(need) * inv(r.rev(), need)).pre(need).rev();
        return *this;
    }
    inline FPS& operator%=(const FPS& r) {
        return *this = divmod(*this, r).second;
    }
    inline FPS operator/(const FPS& r) const { return FPS(*this) /= r; }
    inline FPS operator%(const FPS& r) const { return FPS(*this) %= r; }
    // {f / g, f % g}, g must be normalized (g.back() must not be 0)
    inline friend std::pair<FPS, FPS> divmod(const FPS& f, const FPS& g) {
        assert(!g.empty());
        assert(g.back() != 0);
        FPS q = f / g;
        FPS r = f - q * g;
        return {q, r};
    }

    // log(f) = \int f'/f dx, f[0] must be 1
    inline friend FPS log(const FPS& f, long long deg) {
        assert(!f.empty() && f[0] == 1);
        FPS res = intg(diff(f) * inv(f, deg));
        res.resize(deg);
        return res;
    }
    inline friend FPS log(const FPS& f) {
        return log(f, f.size());
    }

    // exp(f), f[0] must be 0
    inline friend FPS exp(const FPS& f, long long deg) {
        assert(f.empty() || f[0] == 0);
        if (deg <= 0) return FPS();
        if constexpr (internal_convolution::ntt_candidate<mint>()) {
            if (internal_fps::use_ntt<mint>(2 * deg)) return internal_fps::exp_ntt<mint>(f, deg);
        }
        FPS res(1, 1);
        for (long long i = 1; i < deg; i <<= 1) {
            res = res * (f.pre(i << 1) - log(res, i << 1) + 1).pre(i << 1);
        }
        res.resize(deg);
        return res;
    }
    inline friend FPS exp(const FPS& f) {
        return exp(f, f.size());
    }

    // pow(f) = exp(e * log f)
    inline friend FPS pow(const FPS& f, long long e, long long deg) {
        if (e == 0) {
            auto ret = FPS(deg, 0);
            if (deg > 0) ret[0] = 1;
            return ret;
        }
        long long i = 0;
        while (i < static_cast<long long>(f.size()) && f[i] == 0) ++i;
        if (i == static_cast<long long>(f.size())) return FPS(deg, 0);
        if ((i >= 1 && e >= deg) || i * e >= deg) return FPS(deg, 0);
        mint k = f[i];
        FPS res = exp(log((f >> i) / k, deg) * e, deg) * k.pow(e) << (e * i);
        res.resize(deg);
        return res;
    }
    inline friend FPS pow(const FPS& f, long long e) {
        return pow(f, e, f.size());
    }

    // sqrt(f) の先頭 deg 項 (g^2 = f)。平方根がなければ std::nullopt
    // 最初の 0 でない項 f_i x^i は、i が偶数で f_i が平方剰余であること。法は素数であること
    inline friend std::optional<FPS> sqrt(const FPS& f, long long deg) {
        long long i = 0;
        while (i < static_cast<long long>(f.size()) && f[i] == 0) ++i;
        if (i == static_cast<long long>(f.size()) || i / 2 >= deg) return FPS(deg, 0);
        if (i % 2 == 1) return std::nullopt;
        long long r = sqrt_mod(f[i].val(), mint::mod());
        if (r == -1) return std::nullopt;
        // x^i を外した g = f / x^i (g_0 != 0) の平方根をニュートン法で求める: h <- (h + g / h) / 2
        const FPS g = f >> i;
        const long long d = deg - i / 2;
        FPS h({mint(r)});
        const mint inv2 = mint(2).inv();
        for (long long k = 1; k < d; k <<= 1) {
            h = (h + (g.pre(k << 1) * inv(h, k << 1)).pre(k << 1)) * inv2;
        }
        h.resize(d);
        h <<= i / 2;
        return h;
    }
    inline friend std::optional<FPS> sqrt(const FPS& f) {
        return sqrt(f, f.size());
    }

    inline friend FPS taylor_shift(FPS f, mint a) {
        using C = Combination<mint>;
        int n = f.size();
        C::extend(n);
        for (int i = 0; i < n; i++) f[i] *= C::factorial(i);
        std::reverse(f.begin(), f.end());
        FPS<mint> g(n, 1);
        for (int i = 1; i < n; i++) g[i] = g[i - 1] * a * C::inv(i);
        f = (f * g).pre(n);
        std::reverse(f.begin(), f.end());
        for (int i = 0; i < n; i++) f[i] *= C::inverse_factorial(i);
        return f;
    }
};

namespace internal_fps {
template <typename mint>
FPS<mint> shrink(FPS<mint> f) {
    f.normalize();
    return f;
}

// x^k で割って低次の項を捨てる。長さが k 以下なら 0 (空)
template <typename mint>
FPS<mint> drop(const FPS<mint>& f, long long k) {
    if (static_cast<long long>(f.size()) <= k) return FPS<mint>();
    return FPS<mint>(f.begin() + k, f.end());
}

// 行列の成分の積和 Σ_j x[i][j] y[j][k] を、各成分を 1 回ずつ NTT して求める (法が NTT に向き、長さが十分長いときだけ)
// x は rows × 2、y は 2 × cols。結果は rows × cols。使えなければ false を返す
template <typename mint>
bool mat_mul_ntt(const FPS<mint>* const* x, int rows, const FPS<mint>* const* y, int cols, FPS<mint>* res) {
    if constexpr (!internal_convolution::ntt_candidate<mint>()) {
        return false;
    } else {
        long long len = 0, small = 1LL << 60;
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < cols; k++) {
                    const long long s1 = x[i * 2 + j]->size(), s2 = y[j * cols + k]->size();
                    if (s1 == 0 || s2 == 0) continue;
                    len = std::max(len, s1 + s2 - 1), small = std::min(small, std::min(s1, s2));
                }
        // 短いものは普通に掛けたほうが速い
        if (len == 0 || small <= 64 || !use_ntt<mint>(len)) return false;
        long long z = 1;
        while (z < len) z <<= 1;
        auto transform = [&](const FPS<mint>& f) {
            std::vector<mint> t(z);
            std::copy(f.begin(), f.end(), t.begin());
            ntt(t);
            return t;
        };
        std::vector<std::vector<mint>> tx(rows * 2), ty(2 * cols);
        for (int i = 0; i < rows * 2; i++) tx[i] = transform(*x[i]);
        for (int i = 0; i < 2 * cols; i++) ty[i] = transform(*y[i]);
        for (int i = 0; i < rows; i++)
            for (int k = 0; k < cols; k++) {
                std::vector<mint> t(z);
                for (long long e = 0; e < z; e++) t[e] = tx[i * 2][e] * ty[k][e] + tx[i * 2 + 1][e] * ty[cols + k][e];
                intt(t);
                t.resize(len);
                res[i * cols + k] = shrink(FPS<mint>(std::move(t)));
            }
        return true;
    }
}

// 多項式を成分に持つ 2x2 行列 [[a, b], [c, d]]
template <typename mint>
struct Mat {
    FPS<mint> a, b, c, d;
    static Mat identity() { return {FPS<mint>{1}, FPS<mint>(), FPS<mint>(), FPS<mint>{1}}; }
    Mat operator*(const Mat& r) const {
        const FPS<mint>* x[4] = {&a, &b, &c, &d};
        const FPS<mint>* y[4] = {&r.a, &r.b, &r.c, &r.d};
        Mat res;
        FPS<mint>* out[4] = {&res.a, &res.b, &res.c, &res.d};
        FPS<mint> tmp[4];
        if (mat_mul_ntt(x, 2, y, 2, tmp)) {
            for (int i = 0; i < 4; i++) *out[i] = std::move(tmp[i]);
            return res;
        }
        return {shrink(a * r.a + b * r.c), shrink(a * r.b + b * r.d), shrink(c * r.a + d * r.c), shrink(c * r.b + d * r.d)};
    }
    std::pair<FPS<mint>, FPS<mint>> operator*(const std::pair<FPS<mint>, FPS<mint>>& p) const {
        const FPS<mint>* x[4] = {&a, &b, &c, &d};
        const FPS<mint>* y[2] = {&p.first, &p.second};
        FPS<mint> tmp[2];
        if (mat_mul_ntt(x, 2, y, 1, tmp)) return {std::move(tmp[0]), std::move(tmp[1])};
        return {shrink(a * p.first + b * p.second), shrink(c * p.first + d * p.second)};
    }
};

// 互除法を 1 段進める: (p, q) <- (q, p % q)、m <- [[0, 1], [1, -p / q]] m
template <typename mint>
void step(Mat<mint>& m, std::pair<FPS<mint>, FPS<mint>>& p) {
    const FPS<mint> quo = p.first / p.second;
    FPS<mint> rem = shrink(p.first - p.second * quo);
    FPS<mint> c = shrink(m.a - m.c * quo), d = shrink(m.b - m.d * quo);
    m = {std::move(m.c), std::move(m.d), std::move(c), std::move(d)};
    p = {std::move(p.second), std::move(rem)};
}

// p = (f, g) (|f| = n > |g|) に対し、互除法で長さが ceil(n / 2) 以下になる直前までの変換行列
template <typename mint>
Mat<mint> half_gcd(std::pair<FPS<mint>, FPS<mint>> p) {
    const long long n = p.first.size(), m = p.second.size();
    const long long k = (n + 1) / 2;
    if (m <= k) return Mat<mint>::identity();
    // 短いときは、互除法を 1 段ずつ進めたほうが速い
    if (n <= 64) {
        Mat<mint> res = Mat<mint>::identity();
        while (static_cast<long long>(p.second.size()) > k) step(res, p);
        return res;
    }
    // 上位の半分だけで求めた行列は、上位の約半分の商を正しく与える
    Mat<mint> m1 = half_gcd(std::make_pair(drop(p.first, k), drop(p.second, k)));
    p = m1 * p;
    if (static_cast<long long>(p.second.size()) <= k) return m1;
    step(m1, p);
    if (static_cast<long long>(p.second.size()) <= k) return m1;
    const long long l = static_cast<long long>(p.first.size()) - 1;
    const long long j = 2 * k - l;
    p = {drop(p.first, j), drop(p.second, j)};
    return half_gcd(std::move(p)) * m1;
}

// M (f, g) = (gcd(f, g), 0) となる行列 M
template <typename mint>
Mat<mint> gcd_matrix(const FPS<mint>& f, const FPS<mint>& g) {
    std::pair<FPS<mint>, FPS<mint>> p{shrink(f), shrink(g)};
    if (p.first.size() < p.second.size()) {
        Mat<mint> m = gcd_matrix(g, f);
        std::swap(m.a, m.b);
        std::swap(m.c, m.d);
        return m;
    }
    Mat<mint> res = Mat<mint>::identity();
    while (!p.second.empty()) {
        Mat<mint> m1 = half_gcd(p);
        p = m1 * p;
        if (!p.second.empty()) step(m1, p);
        res = m1 * res;
    }
    return res;
}
// 最高次の係数が 1 の gcd。行列は掛け合わせず、組 (p, q) だけを進める
template <typename mint>
FPS<mint> gcd(const FPS<mint>& f, const FPS<mint>& g) {
    std::pair<FPS<mint>, FPS<mint>> p{shrink(f), shrink(g)};
    if (p.first.size() < p.second.size()) std::swap(p.first, p.second);
    while (!p.second.empty()) {
        Mat<mint> m1 = half_gcd(p);
        p = m1 * p;
        if (!p.second.empty()) step(m1, p);
    }
    if (!p.first.empty()) p.first /= p.first.back();
    return p.first;
}
}  // namespace internal_fps

/**
 * @brief f^n mod m (m は末尾が 0 でないこと) O(d log d log n) (d は m の次数)
 */
template <typename mint>
FPS<mint> modpow(const FPS<mint>& f, long long n, const FPS<mint>& m) {
    assert(!m.empty() && m.back() != 0);
    const long long d = static_cast<long long>(m.size()) - 1;  // m の次数
    if (d == 0) return FPS<mint>();
    // h (次数 2d - 2 以下) を m で割った余り。商の次数は d - 2 以下なので、rev(m) の逆元は d - 1 項あればよく、最初に 1 回だけ求める (Barrett)
    const FPS<mint> iv = inv(m.rev(), std::max(d - 1, 1LL));
    auto reduce = [&](FPS<mint> h) {
        h.normalize();
        if (h.size() < m.size()) return h;
        const long long need = static_cast<long long>(h.size()) - static_cast<long long>(m.size()) + 1;
        const FPS<mint> q = (h.rev().pre(need) * iv.pre(need)).pre(need).rev();
        h -= q * m;
        h.resize(std::min<long long>(h.size(), d));
        h.normalize();
        return h;
    };
    const FPS<mint> base = f % m;
    FPS<mint> res(1, 1);
    for (int bit = 62; bit >= 0; bit--) {
        res = reduce(res * res);
        if (n >> bit & 1) res = reduce(res * base);
    }
    return res;
}

/**
 * @brief a f + b g = d (d は f と g の最大公約数で、最高次の係数が 1) となる a、b を求め、d を返す O(n log^2 n)
 * f と g がともに 0 なら d = a = b = 0。a、b は末尾の 0 を取り除いた形
 */
template <typename mint>
FPS<mint> extgcd(const FPS<mint>& f, const FPS<mint>& g, FPS<mint>& a, FPS<mint>& b) {
    using namespace internal_fps;
    Mat<mint> m = gcd_matrix(f, g);
    FPS<mint> d = shrink(m.a * shrink(f) + m.b * shrink(g));
    a = std::move(m.a), b = std::move(m.b);
    if (d.empty()) {
        a.clear(), b.clear();
        return d;
    }
    const mint iv = d.back().inv();
    d *= iv, a *= iv, b *= iv;
    return d;
}

/**
 * @brief f h ≡ 1 (mod g) で、次数が g より小さい h。なければ std::nullopt O(n log^2 n)
 * g は 0 でないこと。g が定数ならすべてが 0 と合同なので、h = 0 (空) を返す。h は末尾の 0 を取り除いた形
 */
template <typename mint>
std::optional<FPS<mint>> modinv(const FPS<mint>& f, const FPS<mint>& g) {
    using namespace internal_fps;
    const FPS<mint> gs = shrink(g);
    assert(!gs.empty());
    if (gs.size() == 1) return FPS<mint>();
    FPS<mint> a, b;
    const FPS<mint> d = extgcd(shrink(f % gs), gs, a, b);
    if (d.size() != 1) return std::nullopt;
    return shrink(a % gs);
}

/**
 * @brief 多項式の列の積 f_0 f_1 ... f_{n-1} O(N log^2 N) (N は次数の和)
 * 次数の小さいものから 2 つずつ掛けていく。fs が空なら 1
 */
template <typename mint>
FPS<mint> polynomial_product(std::vector<FPS<mint>> fs) {
    if (fs.empty()) return FPS<mint>(1, 1);
    auto larger = [&](int a, int b) { return fs[a].size() > fs[b].size(); };
    std::priority_queue<int, std::vector<int>, decltype(larger)> pq(larger);
    for (int i = 0; i < static_cast<int>(fs.size()); i++) pq.push(i);
    while (pq.size() > 1) {
        int a = pq.top();
        pq.pop();
        int b = pq.top();
        pq.pop();
        // 空の多項式 (0) との積は 0
        fs[a] = fs[a].empty() || fs[b].empty() ? FPS<mint>() : fs[a] * fs[b];
        FPS<mint>().swap(fs[b]);
        pq.push(a);
    }
    return fs[pq.top()];
}

/**
 * @brief 分数の和 Σ_i nums[i] / dens[i] を 1 つの分数 num / den にまとめる O(N log^2 N) (N は多項式の長さの和)
 * 半分ずつに分けて a / b + c / d = (ad + cb) / bd とまとめる。den は dens の積。空なら 0 / 1
 * @return std::pair<FPS<mint>, FPS<mint>> {num, den}
 */
template <typename mint>
std::pair<FPS<mint>, FPS<mint>> sum_of_fractions(const std::vector<FPS<mint>>& nums, const std::vector<FPS<mint>>& dens) {
    assert(nums.size() == dens.size());
    if (nums.empty()) return {FPS<mint>(), FPS<mint>{1}};
    auto rec = [&](auto&& self, long long l, long long r) -> std::pair<FPS<mint>, FPS<mint>> {
        if (r - l == 1) return {nums[l], dens[l]};
        long long m = (l + r) / 2;
        auto [a, b] = self(self, l, m);
        auto [c, d] = self(self, m, r);
        return {a * d + c * b, b * d};
    };
    return rec(rec, 0, nums.size());
}
