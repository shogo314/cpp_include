#pragma once

/**
 * @file range-lis.hpp
 * @brief 区間の最長増加部分列
 */

#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>
#include <vector>

#include "wavelet-matrix.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/range-lis.hpp

namespace internal_range_lis {

// 置換 p, q (行 -> 列、長さ n) の unit-Monge 積 r = p ⊡ q O(n log n)
// Σ_P(i, j) = #{i' >= i : p[i'] < j} として、r の Σ は min_j (Σ_P(i, j) + Σ_Q(j, k))
// 内側の添字 j を半分に分けて再帰し、2 つの結果を「どちらの min を使うか」の境界の階段に沿って直す
// w は作業用。1 段で 12n + 2 使い、次の段は切り上げた半分なので、合計 24n + 26 log n 程度あれば足りる
inline void unit_monge_mul(int n, const int* p, const int* q, int* r, int* w) {
    if (n <= 1) {
        if (n == 1) r[0] = 0;
        return;
    }
    const int mid = n / 2;
    // part 0: p[i] < mid の行と j < mid、part 1: p[i] >= mid の行と j >= mid
    int* col[2] = {w, w + n};              // 行 -> 列
    int* row[2] = {w + 2 * n, w + 3 * n};  // 列 -> 行
    int *f = w + 4 * n, *g = w + 5 * n + 1, *id = w + 6 * n + 2;
    int *pp = w + 7 * n + 2, *qq = pp + n, *rr = qq + n, *rows = rr + n, *cols = rows + n, *next = cols + n;
    std::fill(w, w + 4 * n, -1);
    for (int part = 0; part < 2; part++) {
        const int lo = part == 0 ? 0 : mid, hi = part == 0 ? mid : n, m = hi - lo;
        std::fill(id, id + n, -1);
        for (int j = lo; j < hi; j++) id[q[j]] = 0;
        for (int k = 0, c = 0; k < n; k++)
            if (id[k] != -1) id[k] = c, cols[c++] = k;
        for (int i = 0, c = 0; i < n; i++)
            if (lo <= p[i] && p[i] < hi) rows[c] = i, pp[c++] = p[i] - lo;
        for (int j = lo; j < hi; j++) qq[j - lo] = id[q[j]];
        unit_monge_mul(m, pp, qq, rr, next);
        for (int t = 0; t < m; t++) col[part][rows[t]] = cols[rr[t]], row[part][cols[rr[t]]] = rows[t];
    }
    // d(i, k) = (part 1 を使う値) - (part 0 を使う値) は、k が増えると減り、i が増えると減る (1 歩で 0 か 1)
    auto right = [&](int i, int k) { return -static_cast<int>(row[1][k] != -1 && row[1][k] < i) - static_cast<int>(row[0][k] != -1 && row[0][k] >= i); };
    auto up = [&](int i, int k) { return static_cast<int>(col[1][i - 1] != -1 && col[1][i - 1] < k) + static_cast<int>(col[0][i - 1] != -1 && col[0][i - 1] >= k); };
    // f[i]: d(i, k) <= 0 となる最小の k (そこで d = 0)、g[i]: d(i, k) >= 0 となる最大の k
    int kf = 0, df = 0, kg = 0, dg = 0;
    for (int i = n;; i--) {
        while (df > 0) df += right(i, kf), kf++;
        while (kg < n && dg + right(i, kg) >= 0) dg += right(i, kg), kg++;
        f[i] = kf, g[i] = kg;
        if (i == 0) break;
        df += up(i, kf), dg += up(i, kg);
    }
    // 行 i の点は、境界より左上 (d >= 0) にある part 0 の点か、右下 (d <= 0) にある part 1 の点をそのまま使い、
    // どちらも境界をまたぐなら、境界の角 (d(i, k) = 1、d(i + 1, k + 1) = -1 となる k = g[i + 1]) に移す
    for (int i = 0; i < n; i++) {
        if (col[0][i] != -1) {
            r[i] = col[0][i] + 1 <= g[i + 1] ? col[0][i] : g[i + 1];
        } else {
            r[i] = col[1][i] >= f[i] ? col[1][i] : g[i + 1];
        }
    }
}

// 点のない行や列がある部分置換 (行 -> 列、なければ -1) の積
// a の空の行を上に寄せて a の空の列を割り当て、b の空の列を右に寄せて b の空の行を割り当てた置換の積から戻す
inline std::vector<int> subunit_monge_mul(const std::vector<int>& a, const std::vector<int>& b) {
    const int n = a.size();
    std::vector<bool> a_col(n, false), b_col(n, false);
    for (int i = 0; i < n; i++) {
        if (a[i] != -1) a_col[a[i]] = true;
        if (b[i] != -1) b_col[b[i]] = true;
    }
    std::vector<int> ap, a_rows;
    for (int j = 0; j < n; j++)
        if (!a_col[j]) ap.push_back(j);
    const int top = ap.size();
    for (int i = 0; i < n; i++)
        if (a[i] != -1) ap.push_back(a[i]), a_rows.push_back(i);
    std::vector<int> b_cols, pos(n, -1), bp(n);
    for (int k = 0; k < n; k++)
        if (b_col[k]) pos[k] = b_cols.size(), b_cols.push_back(k);
    int cnt = b_cols.size();
    for (int j = 0; j < n; j++) bp[j] = b[j] != -1 ? pos[b[j]] : cnt++;
    std::vector<int> c(n), work(26 * n + 2048);
    unit_monge_mul(n, ap.data(), bp.data(), c.data(), work.data());
    std::vector<int> res(n, -1);
    for (int t = 0; t < static_cast<int>(a_rows.size()); t++) {
        if (c[top + t] < static_cast<int>(b_cols.size())) res[a_rows[t]] = b_cols[c[top + t]];
    }
    return res;
}

// 置換 p の seaweed (Tiskin)。左端の行 i から入る seaweed が右端の行 res[i] から出る (上から入るものは -1)
// 値で半分に分けて再帰し、もう一方の値の位置は素通りさせて、2 つを subunit_monge_mul でつなぐ O(n log^2 n)
inline std::vector<int> seaweed(const std::vector<int>& p) {
    const int n = p.size();
    if (n <= 1) return std::vector<int>(n, -1);
    const int mid = n / 2;
    std::vector<int> lo, hi, lo_pos, hi_pos;
    for (int i = 0; i < n; i++) {
        if (p[i] < mid) {
            lo.push_back(p[i]), lo_pos.push_back(i);
        } else {
            hi.push_back(p[i] - mid), hi_pos.push_back(i);
        }
    }
    lo = seaweed(lo), hi = seaweed(hi);
    std::vector<int> a(n), b(n);
    std::iota(a.begin(), a.end(), 0), std::iota(b.begin(), b.end(), 0);
    for (int i = 0; i < static_cast<int>(lo.size()); i++) a[lo_pos[i]] = lo[i] == -1 ? -1 : lo_pos[lo[i]];
    for (int i = 0; i < static_cast<int>(hi.size()); i++) b[hi_pos[i]] = hi[i] == -1 ? -1 : hi_pos[hi[i]];
    return subunit_monge_mul(a, b);
}

}  // namespace internal_range_lis

/**
 * @brief 固定した列について、区間の最長増加部分列 (狭義) の長さをオンラインで答える
 *
 * seaweed (Tiskin) を値の分割統治と unit-Monge 行列の積で O(n log^2 n) で求めると、
 * LIS(a[l, r)) = (r - l) - #{l <= i < n : 左端の行 i から入った seaweed が右端の行 r 未満から出る} になるので、Wavelet Matrix で数える
 * 構築 O(n log^2 n)、クエリ O(log n)
 * 例:
 * RangeLIS rl(a);
 * long long len = rl.lis(l, r);
 *
 * @tparam T 値の型 (operator< で比べられること)
 */
template <typename T>
class RangeLIS {
    long long n;
    WaveletMatrix<long long> wm;

    static std::vector<long long> build(const std::vector<T>& a) {
        const int n = a.size();
        // 置換に直す。同じ値は後ろのものほど小さくして、狭義の増加列だけが残るようにする
        std::vector<int> idx(n), p(n);
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](int i, int j) { return a[i] < a[j] || (!(a[j] < a[i]) && i > j); });
        for (int k = 0; k < n; k++) p[idx[k]] = k;
        const auto row = internal_range_lis::seaweed(p);
        std::vector<long long> res(n);
        for (int i = 0; i < n; i++) res[i] = row[i] == -1 ? n : row[i];
        return res;
    }

   public:
    /**
     * @brief 列 a から作る
     */
    explicit RangeLIS(const std::vector<T>& a) : n(a.size()), wm(build(a)) {}
    /**
     * @brief 要素数
     */
    long long size() const { return n; }
    /**
     * @brief a[l], ..., a[r-1] の最長増加部分列 (狭義) の長さ
     */
    long long lis(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        return (r - l) - wm.count(l, n, 0, r);
    }
};

template <typename T>
RangeLIS(std::vector<T>) -> RangeLIS<T>;
