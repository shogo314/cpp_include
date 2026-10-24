#pragma once

/**
 * @file fps-composition.hpp
 * @brief 形式的冪級数の合成と、合成についての逆関数 (Kinoshita–Li)
 */

#include <algorithm>
#include <cassert>
#include <vector>

#include "convolution.hpp"
#include "fps.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/math/fps-composition.hpp

namespace internal_fps_composition {
// 2 変数多項式 A(x, y) を、x の次数ごとに y の係数を幅 w で並べた 1 次元の配列で持つ (A[i * w + j] が x^i y^j の係数)
// Q(x, y) = 1 - y g(x) から始め、Q(x, y) Q(-x, y) の x の偶数次だけを残して x の次数を半分にしていく
// qs[t] は t 段目の Q で、x は nx[t] 項、y は 2^t + 1 項。最後の段で nx = 1 になる
template <typename mint>
struct Graeffe {
    std::vector<std::vector<mint>> qs;
    std::vector<long long> nx;
    Graeffe(const FPS<mint>& g, long long n) {
        long long x = n, w = 2;
        std::vector<mint> q(x * w);
        q[0] = 1;
        for (long long i = 0; i < x && i < static_cast<long long>(g.size()); i++) q[i * w + 1] = -g[i];
        while (x > 1) {
            qs.push_back(q);
            nx.push_back(x);
            // y の幅を 2w - 1 に広げて詰め直すと、1 次元の畳み込みが 2 次元の積になる
            const long long nw = 2 * w - 1;
            std::vector<mint> a(x * nw), b(x * nw);
            for (long long i = 0; i < x; i++) {
                for (long long j = 0; j < w; j++) {
                    a[i * nw + j] = q[i * w + j];
                    b[i * nw + j] = i % 2 == 0 ? q[i * w + j] : -q[i * w + j];
                }
            }
            std::vector<mint> c = convolution(a, b);
            const long long x2 = (x + 1) / 2;
            q.assign(x2 * nw, 0);
            for (long long i = 0; i < x2; i++) {
                for (long long j = 0; j < nw; j++) q[i * nw + j] = c[2 * i * nw + j];
            }
            x = x2, w = nw;
        }
    }
    long long depth() const { return qs.size(); }
};

// h_i = Σ_k w_k [x^k] g^i (0 <= i < m) O(n log^2 n) (n = |w|)。g_0 = 0 であること
// [x^{n-1}] W(x) / (1 - y g(x)) (W(x) = Σ_k w_k x^{n-1-k}) を、x について Bostan-Mori 法で求める
template <typename mint>
std::vector<mint> power_projection(const std::vector<mint>& wt, const FPS<mint>& g, long long m) {
    const long long n = wt.size();
    std::vector<mint> res(m);
    if (n == 0) return res;
    Graeffe<mint> gr(g, n);
    // p は t 段目の分子で、x は nx[t] 項、y は 2^t 項
    std::vector<mint> p(n);
    for (long long k = 0; k < n; k++) p[n - 1 - k] = wt[k];
    long long e = 1;
    for (long long t = 0; t < gr.depth(); t++) {
        const long long x = gr.nx[t], w = e + 1, nw = 2 * e;
        const auto& q = gr.qs[t];
        std::vector<mint> a(x * nw), b(x * nw);
        for (long long i = 0; i < x; i++) {
            for (long long j = 0; j < e; j++) a[i * nw + j] = p[i * e + j];
            for (long long j = 0; j < w; j++) b[i * nw + j] = i % 2 == 0 ? q[i * w + j] : -q[i * w + j];
        }
        std::vector<mint> c = convolution(a, b);
        // [x^{x-1}] を取り出すので、x - 1 と同じ偶奇の項だけを残す
        const long long par = (x - 1) % 2, x2 = (x + 1) / 2;
        p.assign(x2 * nw, 0);
        for (long long i = 0; i < x2; i++) {
            for (long long j = 0; j < nw; j++) p[i * nw + j] = c[(2 * i + par) * nw + j];
        }
        e = nw;
    }
    // 最後の段では x が 1 項で、分母の定数項 (x^0) は 1
    for (long long i = 0; i < m && i < e; i++) res[i] = p[i];
    return res;
}

// Σ_i f_i g^i の x^n 未満 O(n log^2 n)。g_0 = 0、|f| <= n であること
// power_projection の転置で、power_projection の各段を逆向きにたどる
template <typename mint>
FPS<mint> composition_zero(const FPS<mint>& f, const FPS<mint>& g, long long n) {
    Graeffe<mint> gr(g, n);
    const long long depth = gr.depth();
    // 最後の段の分子の y の項数は 2^depth
    long long e = 1LL << depth;
    std::vector<mint> p(e);
    for (long long i = 0; i < e && i < static_cast<long long>(f.size()); i++) p[i] = f[i];
    for (long long t = depth - 1; t >= 0; t--) {
        const long long x = gr.nx[t], w = e / 2 + 1, pe = e / 2;
        const auto& q = gr.qs[t];
        // 取り出した偶奇の位置に戻し (r)、Q(-x, y) との積の転置 (両方向を反転した Q(-x, y) との畳み込み) を取る
        const long long par = (x - 1) % 2, x2 = (x + 1) / 2, nw = e + w - 1;
        std::vector<mint> r(x * nw), b(x * nw);
        for (long long i = 0; i < x2; i++) {
            for (long long j = 0; j < e; j++) r[(2 * i + par) * nw + j] = p[i * e + j];
        }
        for (long long i = 0; i < x; i++) {
            for (long long j = 0; j < w; j++) {
                const mint v = q[i * w + j];
                b[(x - 1 - i) * nw + (w - 1 - j)] = i % 2 == 0 ? v : -v;
            }
        }
        std::vector<mint> c = convolution(r, b);
        p.assign(x * pe, 0);
        for (long long i = 0; i < x; i++) {
            for (long long j = 0; j < pe; j++) p[i * pe + j] = c[(i + x - 1) * nw + (j + w - 1)];
        }
        e = pe;
    }
    FPS<mint> res(n);
    for (long long k = 0; k < n; k++) res[k] = p[n - 1 - k];
    return res;
}
}  // namespace internal_fps_composition

/**
 * @brief f(g(x)) の x^deg 未満の項 O(n log^2 n) (n = deg、g_0 != 0 なら O(|f| log |f|) を足す)
 * g が x^v から始まるなら f の次数 ceil(deg / v) 以上の項は使わず、残りが max(32, 4 log2(deg)) 項以下なら Horner 法で O(|f| n log n) にする
 * f は多項式として扱うので、g の定数項は 0 でなくてもよい (f(x + g_0) に直してから求める。|f| は法より小さいこと)。
 * deg を省略するか負の値を渡すと max(|f|, |g|)
 */
template <typename mint>
FPS<mint> composition(FPS<mint> f, FPS<mint> g, long long deg = -1) {
    if (deg < 0) deg = std::max(f.size(), g.size());
    if (deg == 0) return FPS<mint>();
    if (!g.empty() && g[0] != 0) {
        f = taylor_shift(f, g[0]);
        g[0] = 0;
    }
    g = g.pre(deg);
    // g の最低次の項を x^v とすると、g^k は x^{kv} から始まるので、f の k >= ceil(deg / v) の項は効かない
    long long v = 1;
    while (v < static_cast<long long>(g.size()) && g[v] == 0) v++;
    const long long need = v >= static_cast<long long>(g.size()) ? 1 : (deg - 1) / v + 1;
    if (static_cast<long long>(f.size()) > need) f.resize(need);
    // f が短いときは Horner 法 (掛け算 |f| 回) のほうが速い
    // (Kinoshita–Li は長さ deg の掛け算の 4 log2(deg)〜7 log2(deg) 回分ほど。deg = 10^4 で 55 回、2 * 10^5 で 130 回)
    long long log_deg = 0;
    while ((1LL << log_deg) < deg) log_deg++;
    if (static_cast<long long>(f.size()) <= std::max(32LL, 4 * log_deg)) {
        FPS<mint> res(deg);
        for (long long i = static_cast<long long>(f.size()) - 1; i >= 0; i--) {
            res = (res * g).pre(deg);
            res.resize(deg);
            res[0] += f[i];
        }
        return res;
    }
    return internal_fps_composition::composition_zero(f, g, deg);
}

/**
 * @brief f(h(x)) = x となる h の x^deg 未満の項 O(n log^2 n) (n = deg)
 * f_0 = 0、f_1 != 0 であること。deg を省略するか負の値を渡すと |f|
 * N = deg - 1 として、ラグランジュの反転公式 [x^N] f^k = (k / N) [x^{N-k}] (x / h)^N から (x / h)^N を求め、N 乗根を取る
 */
template <typename mint>
FPS<mint> compositional_inverse(const FPS<mint>& f, long long deg = -1) {
    assert(f.size() >= 2 && f[0] == 0 && f[1] != 0);
    if (deg < 0) deg = f.size();
    if (deg <= 2) {
        FPS<mint> h = {0, f[1].inv()};
        h.resize(deg);
        return h;
    }
    const long long N = deg - 1;
    // p_k = [x^N] f^k (0 <= k <= N)
    std::vector<mint> wt(deg);
    wt[N] = 1;
    std::vector<mint> p = internal_fps_composition::power_projection(wt, f.pre(deg), deg);
    // A = (x / h)^N の x^N 未満: [x^j] A = N / (N - j) p_{N-j}
    FPS<mint> A(N);
    for (long long j = 0; j < N; j++) A[j] = p[N - j] * mint(N) / mint(N - j);
    // x / h の定数項は f_1 なので、(x / h) = f_1 (A / f_1^N)^{1/N}
    const mint c = f[1].pow(N);
    FPS<mint> B = exp(log(A / c, N) * mint(N).inv(), N) * f[1];
    return inv(B, N) << 1;
}
