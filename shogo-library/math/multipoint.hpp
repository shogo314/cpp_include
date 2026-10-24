#pragma once

/**
 * @file multipoint.hpp
 * @brief 多点評価・多項式補間・標本点のずらし
 */

#include <algorithm>
#include <cassert>
#include <vector>

#include "combinatorics.hpp"
#include "convolution.hpp"
#include "fps.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/multipoint.hpp

namespace internal_multipoint {
// 点 xs の subproduct tree: tree[k] は頂点 k が受け持つ点 x_i について Π (x - x_i)。葉は sz + i
template <typename mint>
struct SubproductTree {
    long long n, sz;
    std::vector<FPS<mint>> tree;
    explicit SubproductTree(const std::vector<mint>& xs) : n(xs.size()), sz(1) {
        while (sz < n) sz <<= 1;
        tree.assign(2 * sz, FPS<mint>{1});
        for (long long i = 0; i < n; i++) tree[sz + i] = FPS<mint>{-xs[i], 1};
        for (long long k = sz - 1; k >= 1; k--) tree[k] = tree[2 * k] * tree[2 * k + 1];
    }
};
}  // namespace internal_multipoint

/**
 * @brief 多項式 f の、点 xs での値 f(x_0), ..., f(x_{m-1}) O((n + m) log^2 (n + m))
 * subproduct tree の根から、各頂点の多項式で割った余りを下ろしていく。余りが短くなったら直接計算する
 */
template <typename mint>
std::vector<mint> multipoint_evaluation(const FPS<mint>& f, const std::vector<mint>& xs) {
    const long long m = xs.size();
    if (m == 0) return {};
    internal_multipoint::SubproductTree<mint> t(xs);
    std::vector<mint> res(m);
    // 頂点 k に f の余り g が届いたとき、受け持つ点の値を求める
    auto rec = [&](auto&& self, long long k, long long l, long long r, FPS<mint> g) -> void {
        if (l >= m) return;
        // 次数が小さければ Horner 法で直接求める
        if (r - l <= 32) {
            for (long long i = l; i < std::min(r, m); i++) res[i] = g.eval(xs[i]);
            return;
        }
        const long long mid = (l + r) / 2;
        self(self, 2 * k, l, mid, g % t.tree[2 * k]);
        self(self, 2 * k + 1, mid, r, g % t.tree[2 * k + 1]);
    };
    FPS<mint> g = f;
    g.normalize();
    rec(rec, 1, 0, t.sz, g % t.tree[1]);
    return res;
}

/**
 * @brief 点 (x_i, y_i) (x_i は互いに異なる) をすべて通る n - 1 次以下の多項式 (長さ n) O(n log^2 n)
 * g = Π (x - x_i) とすると f = Σ y_i / g'(x_i) Π_{j != i} (x - x_j)。g'(x_i) を多点評価し、和を subproduct tree の下から組み立てる
 */
template <typename mint>
FPS<mint> polynomial_interpolation(const std::vector<mint>& xs, const std::vector<mint>& ys) {
    assert(xs.size() == ys.size());
    const long long n = xs.size();
    if (n == 0) return FPS<mint>();
    internal_multipoint::SubproductTree<mint> t(xs);
    std::vector<mint> d = multipoint_evaluation(diff(t.tree[1]), xs);
    // val[k] = Σ_{頂点 k の点 i} c_i Π_{頂点 k の点 j != i} (x - x_j)、c_i = y_i / g'(x_i)
    std::vector<FPS<mint>> val(2 * t.sz);
    for (long long i = 0; i < t.sz; i++) val[t.sz + i] = i < n ? FPS<mint>{ys[i] / d[i]} : FPS<mint>{};
    for (long long k = t.sz - 1; k >= 1; k--) {
        FPS<mint> a = val[2 * k] * t.tree[2 * k + 1], b = val[2 * k + 1] * t.tree[2 * k];
        if (a.size() < b.size()) a.resize(b.size());
        for (size_t i = 0; i < b.size(); i++) a[i] += b[i];
        val[k] = a;
    }
    FPS<mint> f = val[1];
    f.resize(n);
    return f;
}

namespace internal_multipoint {
// ys = f(0), ..., f(n-1) から f(c), ..., f(c+m-1)。c + k が 0 以上 n 未満の整数と重ならないこと
// f(c+k) = (Π_{j<n} (c+k-j)) Σ_i a_i / (c+k-i)、a_i = y_i / (i! (n-1-i)! (-1)^(n-1-i))
template <typename mint>
std::vector<mint> shift_no_overlap(const std::vector<mint>& ys, mint c, long long m) {
    using C = Combination<mint>;
    const long long n = ys.size();
    C::extend(n);
    std::vector<mint> a(n);
    for (long long i = 0; i < n; i++) {
        a[i] = ys[i] * C::inverse_factorial(i) * C::inverse_factorial(n - 1 - i);
        if ((n - 1 - i) % 2 == 1) a[i] = -a[i];
    }
    // b_t = 1 / (c - (n-1) + t) (t = 0, ..., n+m-2)。逆数はまとめて求める
    const long long len = n + m - 1;
    std::vector<mint> v(len), b(len), pre(len + 1, 1);
    for (long long t = 0; t < len; t++) v[t] = c - mint(n - 1) + mint(t), pre[t + 1] = pre[t] * v[t];
    mint inv_all = pre[len].inv();
    for (long long t = len - 1; t >= 0; t--) b[t] = inv_all * pre[t], inv_all *= v[t];
    std::vector<mint> s = convolution(a, b);
    // Π_{j<n} (c+k-j) = v[k] v[k+1] ... v[k+n-1] をずらしながら求める
    std::vector<mint> res(m);
    mint prod = 1;
    for (long long t = 0; t < n; t++) prod *= v[t];
    for (long long k = 0; k < m; k++) {
        if (k > 0) prod *= v[k + n - 1] * b[k - 1];
        res[k] = prod * s[k + n - 1];
    }
    return res;
}
}  // namespace internal_multipoint

/**
 * @brief n - 1 次以下の多項式 f の f(0), ..., f(n-1) から、f(c), f(c+1), ..., f(c+m-1) を求める O((n + m) log (n + m))
 * 法は n + m より大きい素数であること。c + k が 0 以上 n 未満の整数と重なる k は ys をそのまま使う
 */
template <typename mint>
std::vector<mint> sample_point_shift(const std::vector<mint>& ys, mint c, long long m) {
    const long long n = ys.size();
    std::vector<mint> res(m);
    if (n == 0) return res;
    // (c + k) mod p が [0, n) に入るかで区切り、入らない区間ごとに畳み込みで求める
    const long long p = mint::mod();
    long long k = 0;
    while (k < m) {
        long long x = (c.val() + k) % p;
        if (x < n) {
            res[k] = ys[x];
            k++;
            continue;
        }
        // x + len が p か n + p に届くまでは重ならない (x >= n なので次に重なるのは x が p に戻ったとき)
        long long len = std::min(m - k, p - x);
        auto part = internal_multipoint::shift_no_overlap(ys, mint(x), len);
        for (long long i = 0; i < len; i++) res[k + i] = part[i];
        k += len;
    }
    return res;
}

/**
 * @brief 多項式 f の、等比数列の点 a r^i (0 <= i < m) での値 O((n + m) log (n + m)) (chirp-z 変換)
 * ij = C(i + j, 2) - C(i, 2) - C(j, 2) を使うと f(a r^i) = r^{-C(i, 2)} Σ_j (f_j a^j r^{-C(j, 2)}) r^{C(i + j, 2)} となり、畳み込み 1 回で求まる
 */
template <typename mint>
std::vector<mint> multipoint_evaluation_geometric(const FPS<mint>& f, mint a, mint r, long long m) {
    const long long n = f.size();
    std::vector<mint> res(m);
    if (m == 0 || n == 0) return res;
    if (r == 0) {
        // 点は a, 0, 0, ...
        for (long long j = n - 1; j >= 0; j--) res[0] = res[0] * a + f[j];
        for (long long i = 1; i < m; i++) res[i] = f[0];
        return res;
    }
    // w[k] = r^{C(k, 2)}、iw[k] = r^{-C(k, 2)}
    const long long len = n + m - 1;
    const mint ir = r.inv();
    std::vector<mint> w(len), iw(std::max(n, m));
    mint p = 1;
    w[0] = 1;
    for (long long k = 0; k + 1 < len; k++) w[k + 1] = w[k] * p, p *= r;
    p = 1;
    iw[0] = 1;
    for (long long k = 0; k + 1 < static_cast<long long>(iw.size()); k++) iw[k + 1] = iw[k] * p, p *= ir;
    // A を逆順に並べて畳み込むと、[n - 1 + i] が Σ_j A_j w[i + j] になる
    std::vector<mint> A(n);
    mint aj = 1;
    for (long long j = 0; j < n; j++) A[n - 1 - j] = f[j] * aj * iw[j], aj *= a;
    std::vector<mint> c = convolution(A, w);
    for (long long i = 0; i < m; i++) res[i] = iw[i] * c[n - 1 + i];
    return res;
}

namespace internal_multipoint {
// Π_{j<n} (x - a r^j) O(n log n)
// 前半 h = Π_{j<k} (x - a r^j) から、後半は Π_{j<k} (x - a r^{j+k}) = r^{k^2} h(x r^{-k}) と係数を変えるだけで求まる
template <typename mint>
FPS<mint> geometric_product(mint a, mint r, long long n) {
    if (n == 0) return FPS<mint>{1};
    if (n % 2 == 1) {
        FPS<mint> h = geometric_product(a, r, n - 1);
        return h * FPS<mint>{-a * r.pow(n - 1), 1};
    }
    const long long k = n / 2;
    FPS<mint> h = geometric_product(a, r, k);
    const mint rk = r.pow(k), irk = rk.inv();
    FPS<mint> g = h;
    mint c = rk.pow(k);
    for (auto& x : g) x *= c, c *= irk;
    return h * g;
}
}  // namespace internal_multipoint

/**
 * @brief 等比数列の点 (a r^i, y_i) (0 <= i < n) をすべて通る n - 1 次以下の多項式 (長さ n) O(n log n)
 * a r^i は互いに異なること (n >= 2 なら a != 0、r != 0)
 * f = M Σ_i w_i / (x - x_i) (M = Π (x - x_j)、w_i = y_i / Π_{j != i} (x_i - x_j))。
 * 和の x^k の係数 -a^{-(k+1)} Σ_i w_i r^{-i(k+1)} は等比数列の点での多点評価で求まる
 */
template <typename mint>
FPS<mint> polynomial_interpolation_geometric(mint a, mint r, const std::vector<mint>& ys) {
    const long long n = ys.size();
    if (n == 0) return FPS<mint>();
    if (n == 1) return FPS<mint>{ys[0]};
    assert(a != 0 && r != 0);
    const mint ir = r.inv();
    // pw[k] = r^k、P[k] = Π_{t=1}^{k} (r^t - 1)
    std::vector<mint> pw(n), ipw(n), P(n), iP(n);
    pw[0] = ipw[0] = P[0] = 1;
    for (long long k = 1; k < n; k++) pw[k] = pw[k - 1] * r, ipw[k] = ipw[k - 1] * ir, P[k] = P[k - 1] * (pw[k] - 1);
    assert(P[n - 1] != 0 && "a r^i must be distinct");
    iP[n - 1] = P[n - 1].inv();
    for (long long k = n - 1; k >= 1; k--) iP[k - 1] = iP[k] * (pw[k] - 1);
    // Π_{j != i} (x_i - x_j) = a^{n-1} r^{e_i} P_i (-1)^{n-1-i} P_{n-1-i}、e_i = C(i, 2) + i (n - 1 - i)
    // e_{i+1} - e_i = n - 2 - i なので、r^{-e_i} は順に掛けて求める
    std::vector<mint> W(n);
    const mint ian = a.inv().pow(n - 1);
    mint ire = 1;
    for (long long i = 0; i < n; i++) {
        W[i] = ys[i] * ian * ire * iP[i] * iP[n - 1 - i];
        if ((n - 1 - i) % 2 == 1) W[i] = -W[i];
        if (i + 1 < n) ire *= ipw[n - 2 - i];
    }
    std::vector<mint> s = multipoint_evaluation_geometric(FPS<mint>(W), ir, ir, n);
    const mint ia = a.inv();
    mint c = ia;
    FPS<mint> S(n);
    for (long long k = 0; k < n; k++) S[k] = -c * s[k], c *= ia;
    FPS<mint> f = (internal_multipoint::geometric_product(a, r, n) * S).pre(n);
    f.resize(n);
    return f;
}

/**
 * @brief 重み付きの累乗和 s_k = Σ_i w_i x_i^k (0 <= k < n) O(m log^2 m + n log n) (m は点の個数)
 * Σ_k s_k t^k = Σ_i w_i / (1 - x_i t) なので、分数の和を 1 つの分数にまとめ、分母の逆元を掛ける
 * (多点評価の転置にあたる。点は重なってもよい)
 */
template <typename mint>
std::vector<mint> weighted_power_sums(const std::vector<mint>& xs, const std::vector<mint>& ws, long long n) {
    assert(xs.size() == ws.size());
    std::vector<mint> res(n);
    if (n == 0 || xs.empty()) return res;
    std::vector<FPS<mint>> nums(xs.size()), dens(xs.size());
    for (long long i = 0; i < static_cast<long long>(xs.size()); i++) nums[i] = {ws[i]}, dens[i] = {1, -xs[i]};
    auto [num, den] = sum_of_fractions(nums, dens);
    FPS<mint> s = (num * inv(den, n)).pre(n);
    std::copy(s.begin(), s.end(), res.begin());
    return res;
}
