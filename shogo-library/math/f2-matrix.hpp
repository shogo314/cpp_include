#pragma once
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/math/f2-matrix.hpp

/**
 * @file f2-matrix.hpp
 * @brief F2 (mod 2) の行列
 */

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief F2 (mod 2) の行列
 *
 * 各行を 64 bit ずつ詰めて持ち、足し算 (xor)・積・消去を 64 要素ずつまとめて行う
 * ベクトルは std::vector<bool> で受け渡す
 * 例:
 * F2Matrix A(std::vector<std::string>{"110", "011"});
 * auto sol = A.solve({true, false});  // Ax = b
 * A.rank(); A.kernel();
 */
class F2Matrix {
   public:
    long long n, m;  //!< 行数、列数

   private:
    long long w;                   // 1 行の 64 bit の個数
    std::vector<std::uint64_t> a;  // a[i * w + j / 64] の j % 64 bit 目が (i, j) 成分 (行が多くても余計な領域を使わないよう 1 本の配列にする)

    // h 行、1 行 width 個の 64 bit の配列 d を、先頭から cols 列を使って簡約化し、各行の主成分の列を返す
    // (列が cols より多い部分も一緒に変形する)
    static std::vector<long long> reduce(std::vector<std::uint64_t>& d, long long h, long long width, long long cols) {
        std::vector<long long> pivots;
        for (long long c = 0, r = 0; c < cols && r < h; c++) {
            const long long q = c >> 6;
            const std::uint64_t bit = std::uint64_t(1) << (c & 63);
            long long p = r;
            while (p < h && !(d[p * width + q] & bit)) p++;
            if (p == h) continue;
            if (p != r) std::swap_ranges(d.begin() + r * width, d.begin() + (r + 1) * width, d.begin() + p * width);
            const std::uint64_t* pr = d.data() + r * width;
            for (long long i = 0; i < h; i++) {
                std::uint64_t* row = d.data() + i * width;
                if (i == r || !(row[q] & bit)) continue;
                for (long long k = q; k < width; k++) row[k] ^= pr[k];
            }
            pivots.push_back(c);
            r++;
        }
        return pivots;
    }
    static bool get_bit(const std::vector<std::uint64_t>& d, long long width, long long i, long long j) { return d[i * width + (j >> 6)] >> (j & 63) & 1; }
    // 簡約化した d (1 行 width 個、先頭 m 列) から、Ax = 0 の解空間の基底を作る
    std::vector<std::vector<bool>> kernel_from_reduced(const std::vector<std::uint64_t>& d, long long width, const std::vector<long long>& pivots) const {
        std::vector<bool> is_pivot(m, false);
        for (long long c : pivots) is_pivot[c] = true;
        std::vector<std::vector<bool>> res;
        for (long long f = 0; f < m; f++) {
            if (is_pivot[f]) continue;
            std::vector<bool> x(m, false);
            x[f] = true;
            for (long long i = 0; i < static_cast<long long>(pivots.size()); i++) x[pivots[i]] = get_bit(d, width, i, f);
            res.push_back(x);
        }
        return res;
    }

   public:
    /**
     * @brief 連立一次方程式 Ax = b の解
     * 解全体は x + (basis の一部の xor)
     */
    struct Solution {
        std::vector<bool> x;                   //!< 解の 1 つ (特殊解)
        std::vector<std::vector<bool>> basis;  //!< Ax = 0 の解空間の基底
    };

    /**
     * @brief 0 行 0 列の行列
     */
    F2Matrix() : F2Matrix(0, 0) {}
    /**
     * @brief n 行 m 列の零行列
     */
    F2Matrix(long long n, long long m) : n(n), m(m), w((m + 63) / 64), a(n * w, 0) {}
    /**
     * @brief '0' と '1' の文字列の配列から作る (各文字列が 1 行)
     */
    explicit F2Matrix(const std::vector<std::string>& s) : F2Matrix(s.size(), s.empty() ? 0 : s[0].size()) {
        for (long long i = 0; i < n; i++) {
            assert(static_cast<long long>(s[i].size()) == m);
            for (long long j = 0; j < m; j++) set(i, j, s[i][j] == '1');
        }
    }
    /**
     * @brief 整数の 2 次元配列から作る (各成分の偶奇)
     */
    explicit F2Matrix(const std::vector<std::vector<int>>& v) : F2Matrix(v.size(), v.empty() ? 0 : v[0].size()) {
        for (long long i = 0; i < n; i++) {
            assert(static_cast<long long>(v[i].size()) == m);
            for (long long j = 0; j < m; j++) set(i, j, v[i][j] & 1);
        }
    }
    /**
     * @brief n 次の単位行列
     */
    static F2Matrix identity(long long n) {
        F2Matrix I(n, n);
        for (long long i = 0; i < n; i++) I.set(i, i, true);
        return I;
    }

    /**
     * @brief (i, j) 成分
     */
    bool get(long long i, long long j) const {
        assert(0 <= i && i < n && 0 <= j && j < m);
        return get_bit(a, w, i, j);
    }
    /**
     * @brief (i, j) 成分を x にする
     */
    void set(long long i, long long j, bool x) {
        assert(0 <= i && i < n && 0 <= j && j < m);
        const std::uint64_t bit = std::uint64_t(1) << (j & 63);
        if (x) {
            a[i * w + (j >> 6)] |= bit;
        } else {
            a[i * w + (j >> 6)] &= ~bit;
        }
    }

    friend bool operator==(const F2Matrix& A, const F2Matrix& B) { return A.n == B.n && A.m == B.m && A.a == B.a; }
    friend bool operator!=(const F2Matrix& A, const F2Matrix& B) { return !(A == B); }

    /**
     * @brief 足し算 (成分ごとの xor) O(nm / 64)
     */
    F2Matrix& operator+=(const F2Matrix& B) {
        assert(n == B.n && m == B.m);
        for (size_t k = 0; k < a.size(); k++) a[k] ^= B.a[k];
        return *this;
    }
    friend F2Matrix operator+(F2Matrix A, const F2Matrix& B) { return A += B; }
    /**
     * @brief 積 O(nmk / 64) (k は B の列数)
     */
    friend F2Matrix operator*(const F2Matrix& A, const F2Matrix& B) {
        assert(A.m == B.n);
        F2Matrix C(A.n, B.m);
        for (long long i = 0; i < A.n; i++) {
            std::uint64_t* c = C.a.data() + i * C.w;
            // A の i 行目で 1 の列 j ごとに、B の j 行目を足す
            for (long long q = 0; q < A.w; q++) {
                for (std::uint64_t x = A.a[i * A.w + q]; x; x &= x - 1) {
                    const std::uint64_t* row = B.a.data() + ((q << 6) + __builtin_ctzll(x)) * B.w;
                    for (long long k = 0; k < C.w; k++) c[k] ^= row[k];
                }
            }
        }
        return C;
    }
    F2Matrix& operator*=(const F2Matrix& B) { return *this = *this * B; }
    /**
     * @brief 行列と列ベクトルの積 O(nm / 64)
     */
    friend std::vector<bool> operator*(const F2Matrix& A, const std::vector<bool>& x) {
        assert(static_cast<long long>(x.size()) == A.m);
        std::vector<std::uint64_t> xb(A.w, 0);
        for (long long j = 0; j < A.m; j++)
            if (x[j]) xb[j >> 6] |= std::uint64_t(1) << (j & 63);
        std::vector<bool> res(A.n);
        for (long long i = 0; i < A.n; i++) {
            std::uint64_t s = 0;
            for (long long k = 0; k < A.w; k++) s ^= A.a[i * A.w + k] & xb[k];
            res[i] = __builtin_parityll(s);
        }
        return res;
    }
    /**
     * @brief k 乗 (正方行列) O(n^3 / 64 log k)
     */
    F2Matrix pow(long long k) const {
        assert(n == m && k >= 0);
        F2Matrix res = identity(n), b = *this;
        for (; k > 0; k >>= 1) {
            if (k & 1) res *= b;
            b *= b;
        }
        return res;
    }
    /**
     * @brief 転置 O(nm)
     */
    F2Matrix transpose() const {
        F2Matrix T(m, n);
        for (long long i = 0; i < n; i++)
            for (long long j = 0; j < m; j++)
                if (get(i, j)) T.set(j, i, true);
        return T;
    }

    /**
     * @brief 簡約化した行列と、各行の主成分の列 (個数がランク) O(nm min(n, m) / 64)
     */
    std::pair<F2Matrix, std::vector<long long>> reduced_row_echelon() const {
        F2Matrix A = *this;
        std::vector<long long> pivots = reduce(A.a, n, w, m);
        return {A, pivots};
    }
    /**
     * @brief ランク O(nm min(n, m) / 64)
     */
    long long rank() const {
        auto d = a;
        return reduce(d, n, w, m).size();
    }
    /**
     * @brief 行列式 (正方行列) O(n^3 / 64)
     */
    bool det() const {
        assert(n == m);
        return rank() == n;
    }
    /**
     * @brief 逆行列 (正方行列)。正則でなければ std::nullopt O(n^3 / 64)
     */
    std::optional<F2Matrix> inv() const {
        assert(n == m);
        // [A | I] を A の列だけで簡約化すると [I | A^-1] になる
        const long long w2 = (2 * n + 63) / 64;
        std::vector<std::uint64_t> d(n * w2, 0);
        for (long long i = 0; i < n; i++) {
            for (long long k = 0; k < w; k++) d[i * w2 + k] = a[i * w + k];
            d[i * w2 + ((n + i) >> 6)] |= std::uint64_t(1) << ((n + i) & 63);
        }
        if (static_cast<long long>(reduce(d, n, w2, n).size()) < n) return std::nullopt;
        F2Matrix res(n, n);
        for (long long i = 0; i < n; i++)
            for (long long j = 0; j < n; j++)
                if (get_bit(d, w2, i, n + j)) res.set(i, j, true);
        return res;
    }
    /**
     * @brief Ax = 0 の解空間の基底 (長さ m のベクトルを m - rank 個) O(nm min(n, m) / 64)
     */
    std::vector<std::vector<bool>> kernel() const {
        auto d = a;
        std::vector<long long> pivots = reduce(d, n, w, m);
        return kernel_from_reduced(d, w, pivots);
    }
    /**
     * @brief 連立一次方程式 Ax = b を解く。解がなければ std::nullopt O(nm min(n, m) / 64)
     */
    std::optional<Solution> solve(const std::vector<bool>& b) const {
        assert(static_cast<long long>(b.size()) == n);
        // 拡大係数行列 [A | b] を、A の列だけで簡約化する
        const long long w2 = (m + 1 + 63) / 64;
        std::vector<std::uint64_t> d(n * w2, 0);
        for (long long i = 0; i < n; i++) {
            for (long long k = 0; k < w; k++) d[i * w2 + k] = a[i * w + k];
            if (b[i]) d[i * w2 + (m >> 6)] |= std::uint64_t(1) << (m & 63);
        }
        std::vector<long long> pivots = reduce(d, n, w2, m);
        const long long r = pivots.size();
        for (long long i = r; i < n; i++) {
            if (get_bit(d, w2, i, m)) return std::nullopt;
        }
        Solution sol;
        sol.x.assign(m, false);
        for (long long i = 0; i < r; i++) sol.x[pivots[i]] = get_bit(d, w2, i, m);
        sol.basis = kernel_from_reduced(d, w2, pivots);
        return sol;
    }

    /**
     * @brief 各行を '0' と '1' の文字列にしたもの
     */
    std::vector<std::string> to_strings() const {
        std::vector<std::string> res(n, std::string(m, '0'));
        for (long long i = 0; i < n; i++)
            for (long long j = 0; j < m; j++)
                if (get(i, j)) res[i][j] = '1';
        return res;
    }
    /**
     * @brief 各行を '0' と '1' の文字列にして、改行区切りで出力する
     */
    friend std::ostream& operator<<(std::ostream& os, const F2Matrix& A) {
        auto s = A.to_strings();
        for (long long i = 0; i < A.n; i++) {
            if (i != 0) os << '\n';
            os << s[i];
        }
        return os;
    }
};
