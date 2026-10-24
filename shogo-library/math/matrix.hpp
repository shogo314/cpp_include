#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/matrix.hpp

/**
 * @file matrix.hpp
 * @brief 行列
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#if __cplusplus >= 202002L
#include <concepts>
#endif
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <valarray>
#include <vector>

namespace matrix {
template <typename T>
struct OperatorPropertyDefault {
    constexpr static T zero() { return T(0); }
    constexpr static T add(const T& a, const T& b) { return a + b; }
    constexpr static T neg(const T& a) { return -a; }
    constexpr static T one() { return T(1); }
    constexpr static T mul(const T& a, const T& b) { return a * b; }
    constexpr static T inv(const T& a) { return T(1) / a; }
};
}  // namespace matrix

namespace internal_matrix {
// 法が 2^31 未満の modint (mod() と val() を持つ型) か
#if __cplusplus >= 202002L
template <class T>
concept modint_like = requires(const T& x) {
    { T::mod() } -> std::convertible_to<std::uint32_t>;
    { x.val() } -> std::convertible_to<std::uint32_t>;
};
template <class T>
inline constexpr bool is_modint_like_v = modint_like<T>;
#else
template <class T, class = void>
struct is_modint_like : std::false_type {};
template <class T>
struct is_modint_like<T, std::void_t<decltype(static_cast<std::uint32_t>(T::mod())),
                                     decltype(static_cast<std::uint32_t>(std::declval<const T&>().val()))>>
    : std::true_type {};
template <class T>
inline constexpr bool is_modint_like_v = is_modint_like<T>::value;
#endif
}  // namespace internal_matrix

/**
 * @brief 行列
 * @tparam T 型 行列(グリッド)の要素となるintやchar
 * @tparam OperatorProperty 行列の要素の演算を定義する構造体 0,1と+,*なら省略可
 *
 * @note
 * OperatorPropertyの例（整数のxorとandによる環）
 * @code
 * struct XorOperatorProperty {
 *     constexpr static int zero() { return 0; }
 *     constexpr static int add(const int &a, const int &b) { return a ^ b; }
 *     constexpr static int neg(const int &a) { return a; }
 *     constexpr static int one() { return 1; }
 *     constexpr static int mul(const int &a, const int &b) { return a & b; }
 * };
 * @endcode
 * constexprである必要はなく、またinvなど使わないものは定義しなくてもよい（必要なものがなかったらコンパイルエラーが発生する）
 */
template <class T, class OperatorProperty = matrix::OperatorPropertyDefault<T>>
class Matrix {
   public:
    long long n, m;
    std::vector<std::vector<T>> v;

    /**
     * @brief コンストラクタ
     * @param v 行列(グリッド)の元となる vector<string> や vector<vector<T>>
     * @return Matrix
     */
    template <typename Iterable>
    Matrix(const std::vector<Iterable>& v_) noexcept : n(v_.size()), m(v_.size() == 0 ? 0 : v_[0].size()) {
        v.resize(n);
        for (long long i = 0; i < n; i++) {
            v[i].assign(v_[i].begin(), v_[i].end());
        }
    }

    /**
     * @brief 初期化子リストから作るコンストラクタ
     * 例: Matrix<mint> A({{1, 1}, {1, 0}});
     * @param il 各行の要素のリスト
     */
    Matrix(std::initializer_list<std::initializer_list<T>> il) : n(il.size()), m(il.size() == 0 ? 0 : il.begin()->size()) {
        v.reserve(n);
        for (const auto& row : il) v.emplace_back(row);
    }
    /**
     * @brief コンストラクタ
     * @param _n 行列(グリッド)の行数
     * @param _m 行列(グリッド)の列数
     * @param _val 行列(グリッド)の要素の初期値
     * @return Matrix
     */
    Matrix(long long _n, long long _m, T _val = OperatorProperty::zero()) : n(_n), m(_m), v(n, std::vector<T>(m, _val)) {}

    auto begin() noexcept { return v.begin(); }
    auto end() noexcept { return v.end(); }
    auto begin() const noexcept { return v.begin(); }
    auto end() const noexcept { return v.end(); }

    /**
     * @brief 行列(グリッド)の行数
     * @return long long
     */
    [[nodiscard]]
    long long size() const { return v.size(); }

    std::vector<T>& operator[](long long i) { return v[i]; }
    const std::vector<T>& operator[](long long i) const { return v[i]; }
    Matrix& operator=(const std::vector<std::vector<T>>& A) noexcept {
        n = A.size();
        m = (n == 0 ? 0 : A[0].size());
        v = A;
        return *this;
    }
    bool operator==(const Matrix& A) const noexcept {
        return this->v == A.v;
    }
    bool operator!=(const Matrix& A) const noexcept {
        return this->v != A.v;
    }

    /**
     * @brief 転置
     * @return Matrix
     */
    [[nodiscard]]
    Matrix transpose() const noexcept {
        if (n == 0) return Matrix(v);
        std::vector<std::vector<T>> ret(m);
        for (long long i = 0; i < m; i++) {
            ret[i].resize(n);
            for (long long j = 0; j < n; j++) ret[i][j] = v[j][i];
        }
        return Matrix(ret);
    }

    /**
     * @brief 左右反転
     * @return Matrix
     */
    [[nodiscard]]
    Matrix rev_lr() const noexcept {
        std::vector<std::vector<T>> ret = v;
        for (long long i = 0; i < n; i++) std::reverse(ret[i].begin(), ret[i].end());
        return Matrix(ret);
    }

    /**
     * @brief 上下反転
     * @return Matrix
     */
    [[nodiscard]]
    Matrix rev_ud() const noexcept {
        std::vector<std::vector<T>> ret = v;
        reverse(ret.begin(), ret.end());
        return Matrix(ret);
    }

    /**
     * @brief 時計周りに90度回転
     * @param k 回転する回数
     * @return Matrix
     */
    [[nodiscard]]
    Matrix rotate(long long k) const noexcept {
        k %= 4;
        if (k == 0) return *this;
        if (k < 0) k += 4;
        if (k == 2) return this->rev_lr().rev_ud();
        std::vector<std::vector<T>> ret(m);
        if (k == 1) {
            for (long long i = 0; i < m; i++) {
                ret[i].resize(n);
                for (long long j = 0; j < n; j++) ret[i][j] = v[n - j - 1][i];
            }
        } else {
            for (long long i = 0; i < m; i++) {
                ret[i].resize(n);
                for (long long j = 0; j < n; j++) ret[i][j] = v[j][m - i - 1];
            }
        }
        return Matrix(ret);
    }

    /**
     * @brief (i, j)を((i + dy) mod n, (j + dx) mod m)に移動
     * @return Matrix
     */
    [[nodiscard]]
    Matrix shift(long long dy, long long dx) const noexcept {
        if (n == 0 || m == 0) return *this;
        dy = (dy % n + n) % n;
        dx = (dx % m + m) % m;
        std::vector<std::vector<T>> ret = v;
        for (long long i = 0, ni = dy; i < n; i++, ni++) {
            if (ni >= n) ni = 0;
            for (long long j = 0, nj = dx; j < m; j++, nj++) {
                if (nj >= m) nj = 0;
                ret[ni][nj] = v[i][j];
            }
        }
        return Matrix(ret);
    }

    /**
     * @brief 左にk回シフト
     * @return Matrix
     */
    [[nodiscard]]
    Matrix shift_l(long long k) const noexcept {
        return this->shift(0, -k);
    }

    /**
     * @brief 右にk回シフト
     * @return Matrix
     */
    [[nodiscard]]
    Matrix shift_r(long long k) const noexcept {
        return this->shift(0, k);
    }

    /**
     * @brief 上にk回シフト
     * @return Matrix
     */
    [[nodiscard]]
    Matrix shift_u(long long k) const noexcept {
        return this->shift(-k, 0);
    }

    /**
     * @brief 下にk回シフト
     * @return Matrix
     */
    [[nodiscard]]
    Matrix shift_d(long long k) const noexcept {
        return this->shift(k, 0);
    }

    /**
     * @brief グリッドをvector<string>で返す
     * @return std::vector<std::string>
     */
    [[nodiscard]]
    std::vector<std::string> vstr() const noexcept {
        std::vector<std::string> ret(n);
        for (long long i = 0; i < n; i++) {
            ret[i].assign(v[i].begin(), v[i].end());
        }
        return ret;
    }

    /**
     * @brief グリッドのj列目を返す
     * @param j 返す列番号(0-indexed)
     * @return std::vector<T>
     */
    [[nodiscard]]
    std::vector<T> col(long long j) const noexcept {
        std::vector<T> ret(n);
        for (long long i = 0; i < n; i++) {
            ret[i] = v[i][j];
        }
        return ret;
    }

    /**
     * @brief グリッドのi行目をstringで返す
     * @param i 返す行番号(0-indexed)
     * @return std::string
     */
    [[nodiscard]]
    std::string str(long long i) const noexcept {
        std::string ret;
        ret.assign(v[i].begin(), v[i].end());
        return ret;
    }
    /**
     * @brief 保持している行列に行列Bを足す
     * @param B 足す行列
     * @return @c *this
     */
    Matrix& operator+=(const Matrix& B) {
        if (n == 0) return (*this);
        assert(n == B.n && m == B.m);
        for (long long i = 0; i < n; i++)
            for (long long j = 0; j < m; j++)
                (*this)[i][j] = OperatorProperty::add((*this)[i][j], B[i][j]);
        return (*this);
    }
    /**
     * @brief 保持している行列から行列Bを引く
     * @param B 引く行列
     * @return @c *this
     */
    Matrix& operator-=(const Matrix& B) {
        if (n == 0) return (*this);
        assert(n == B.n && m == B.m);
        for (long long i = 0; i < n; i++)
            for (long long j = 0; j < m; j++)
                (*this)[i][j] = OperatorProperty::add((*this)[i][j], OperatorProperty::neg(B[i][j]));
        return (*this);
    }

    /**
     * @brief 保持している行列に行列Bを掛ける
     * @param B 掛ける行列
     * @return @c *this
     */
    Matrix& operator*=(const Matrix& B) {
        assert(m == B.n);
        const long long p = B.m;
        if constexpr (std::is_same_v<OperatorProperty, matrix::OperatorPropertyDefault<T>> &&
                      internal_matrix::is_modint_like_v<T>) {
            // 積を 64 ビットのまま足し、あふれる前にだけ mod を取る
            // 法が 2^30 程度までなら 4 行ずつまとめて足し、acc の読み書きを減らす
            const std::uint64_t mod = T::mod();
            const std::uint64_t lim = UINT64_MAX / ((mod - 1) * (mod - 1) + 1) - 1;
            std::vector<std::uint32_t> b(m * p);
            for (long long k = 0; k < m; k++)
                for (long long j = 0; j < p; j++) b[k * p + j] = B[k][j].val();
            std::vector<std::uint64_t> acc(p);
            for (long long i = 0; i < n; i++) {
                std::fill(acc.begin(), acc.end(), 0);
                std::uint64_t cnt = 0;
                long long k = 0;
                if (lim >= 4) {
                    for (; k + 4 <= m; k += 4) {
                        if (cnt + 4 > lim) {
                            cnt = 0;
                            for (long long j = 0; j < p; j++) acc[j] %= mod;
                        }
                        cnt += 4;
                        const auto& r = (*this)[i];
                        const std::uint64_t a0 = r[k].val(), a1 = r[k + 1].val(), a2 = r[k + 2].val(), a3 = r[k + 3].val();
                        const std::uint32_t *b0 = b.data() + k * p, *b1 = b0 + p, *b2 = b1 + p, *b3 = b2 + p;
                        for (long long j = 0; j < p; j++) acc[j] += a0 * b0[j] + a1 * b1[j] + a2 * b2[j] + a3 * b3[j];
                    }
                }
                for (; k < m; k++) {
                    if (cnt + 1 > lim) {
                        cnt = 0;
                        for (long long j = 0; j < p; j++) acc[j] %= mod;
                    }
                    cnt++;
                    const std::uint64_t a0 = (*this)[i][k].val();
                    const std::uint32_t* b0 = b.data() + k * p;
                    for (long long j = 0; j < p; j++) acc[j] += a0 * b0[j];
                }
                v[i].resize(p);
                for (long long j = 0; j < p; j++) v[i][j] = T(acc[j] % mod);
            }
            m = p;
            return (*this);
        }
        Matrix C(n, p);
        for (long long i = 0; i < n; i++) {
            for (long long k = 0; k < m; k++) {
                for (long long j = 0; j < p; j++) {
                    C[i][j] = OperatorProperty::add(C[i][j], OperatorProperty::mul((*this)[i][k], B[k][j]));
                }
            }
        }
        v.swap(C.v);
        m = p;
        return (*this);
    }

    /**
     * @brief 保持している行列のk乗を求める
     * @param k 指数
     * @return Matrix
     */
    [[nodiscard]]
    Matrix pow(long long k) const {
        Matrix A = *this, B(n, n);
        for (long long i = 0; i < n; i++) B[i][i] = OperatorProperty::one();
        while (k > 0) {
            if (k & 1) B *= A;
            A *= A;
            k >>= 1;
        }
        return B;
    }

    [[nodiscard]]
    Matrix operator+(const Matrix& B) const { return (Matrix(*this) += B); }
    [[nodiscard]]
    Matrix operator-(const Matrix& B) const { return (Matrix(*this) -= B); }
    [[nodiscard]]
    Matrix operator*(const Matrix& B) const { return (Matrix(*this) *= B); }

    /**
     * @brief 行列Aと列ベクトルBの積
     * @param A Matrix
     * @param B vector<T>
     * @return vector<T> 列ベクトル
     */
    [[nodiscard]]
    friend std::vector<T> operator*(const Matrix& A, const std::vector<T>& B) {
        assert(A.m == static_cast<long long>(B.size()));
        std::vector<T> ret(A.n, OperatorProperty::zero());
        for (long long i = 0; i < A.n; i++) {
            for (long long j = 0; j < A.m; j++) {
                ret[i] = OperatorProperty::add(ret[i], OperatorProperty::mul(A[i][j], B[j]));
            }
        }
        return ret;
    }

    /**
     * @brief 行ベクトルAと行列Bの積
     * @param A vector<T>
     * @param B Matrix
     * @return vector<T> 行ベクトル
     */
    [[nodiscard]]
    friend std::vector<T> operator*(const std::vector<T>& A, const Matrix& B) {
        assert(static_cast<long long>(A.size()) == B.n);
        std::vector<T> ret(B.m, OperatorProperty::zero());
        for (long long i = 0; i < B.n; i++) {
            for (long long j = 0; j < B.m; j++) {
                ret[j] = OperatorProperty::add(ret[j], OperatorProperty::mul(A[i], B[i][j]));
            }
        }
        return ret;
    }

    /**
     * @brief 行列式
     * @return 行列式
     * @note 割り算を使うので、要素は体 (modint や浮動小数点型など) であること。
     *       int や long long では割り算が切り捨てになり、正しい値にならない
     */
    [[nodiscard]]
    T det() const {
        assert(n == m);
        if (n == 0) return T(1);
        if constexpr (std::is_same_v<OperatorProperty, matrix::OperatorPropertyDefault<T>>) {
            std::vector A(n, std::valarray(T(0), n));
            for (long long i = 0; i < n; i++)
                for (long long j = 0; j < n; j++) A[i][j] = this->v[i][j];
            return forward_elimination(A);
        } else {
            auto A = this->v;
            return forward_elimination(A);
        }
    }

    /**
     * @brief 行列式を mod で割った余り (mod は素数でなくてよい) O(n^2 (n + log mod))
     * @note 要素は整数型であること。割り算の代わりに、2 つの行で互除法のように引き合って 0 を作る
     */
    [[nodiscard]]
    long long det_mod(long long mod) const {
        assert(n == m);
        assert(1 <= mod && mod <= (1LL << 31));
        std::vector<std::vector<long long>> a(n, std::vector<long long>(n));
        for (long long i = 0; i < n; i++)
            for (long long j = 0; j < n; j++) a[i][j] = ((static_cast<long long>(this->v[i][j]) % mod) + mod) % mod;
        long long res = 1 % mod;
        for (long long i = 0; i < n; i++) {
            for (long long j = i + 1; j < n; j++) {
                // a[j][i] が 0 になるまで、行 i から行 j の q 倍を引いて入れ替える
                while (a[j][i] != 0) {
                    const long long q = a[i][i] / a[j][i];
                    if (q != 0) {
                        for (long long k = i; k < n; k++) a[i][k] = ((a[i][k] - q * a[j][k]) % mod + mod) % mod;
                    }
                    std::swap(a[i], a[j]);
                    res = (mod - res) % mod;
                }
            }
            res = res * a[i][i] % mod;
            if (res == 0) return 0;
        }
        return res;
    }

    /**
     * @brief 特性多項式 det(xI - A) の係数 (低次から n + 1 個、最高次は 1) O(n^3)
     * @note 割り算を使うので、要素は体 (modint など) であること
     *
     * 相似変換で上ヘッセンベルク行列 (i > j + 1 で 0) にしてから、左上の k × k の特性多項式を k の小さい方から求める
     */
    [[nodiscard]]
    std::vector<T> characteristic_polynomial() const {
        assert(n == m);
        using P = OperatorProperty;
        auto sub = [](const T& a, const T& b) { return P::add(a, P::neg(b)); };
        auto H = this->v;
        for (long long j = 0; j + 2 < n; j++) {
            long long piv = j + 1;
            while (piv < n && H[piv][j] == P::zero()) piv++;
            if (piv == n) continue;
            if (piv != j + 1) {
                std::swap(H[piv], H[j + 1]);
                for (long long i = 0; i < n; i++) std::swap(H[i][piv], H[i][j + 1]);
            }
            const T iv = P::inv(H[j + 1][j]);
            for (long long k = j + 2; k < n; k++) {
                const T c = P::mul(H[k][j], iv);
                if (c == P::zero()) continue;
                // 行 k から行 j + 1 の c 倍を引き、列 j + 1 に列 k の c 倍を足す (相似変換)
                for (long long l = 0; l < n; l++) H[k][l] = sub(H[k][l], P::mul(c, H[j + 1][l]));
                for (long long l = 0; l < n; l++) H[l][j + 1] = P::add(H[l][j + 1], P::mul(c, H[l][k]));
            }
        }
        // p[k]: 左上の k × k の特性多項式
        // p[k + 1] = (x - H[k][k]) p[k] - Σ_{i < k} (H[i + 1][i] ... H[k][k - 1]) H[i][k] p[i]
        std::vector<std::vector<T>> p(n + 1);
        p[0] = {P::one()};
        for (long long k = 0; k < n; k++) {
            std::vector<T> q(k + 2, P::zero());
            for (long long d = 0; d <= k; d++) {
                q[d + 1] = P::add(q[d + 1], p[k][d]);
                q[d] = sub(q[d], P::mul(H[k][k], p[k][d]));
            }
            T prod = P::one();
            for (long long i = k - 1; i >= 0; i--) {
                prod = P::mul(prod, H[i + 1][i]);
                if (prod == P::zero()) break;
                const T c = P::mul(prod, H[i][k]);
                for (long long d = 0; d <= i; d++) q[d] = sub(q[d], P::mul(c, p[i][d]));
            }
            p[k + 1] = std::move(q);
        }
        return p[n];
    }

    /**
     * @brief パーマネント Σ_σ Π_i a[i][σ(i)] (Ryser の公式) O(2^n n)
     * @return パーマネント
     * @note 割り算を使わないので、要素は可換環 (long long や modint など) でよい。
     *       0 と 1 の行列なら、二部グラフの完全マッチングの個数になる
     *
     * perm(A) = Σ_{S ⊆ 列} (-1)^{n - |S|} Π_i Σ_{j ∈ S} a[i][j] を、S をグレイコードの順に動かして各行の和を O(n) で更新する
     */
    [[nodiscard]]
    T permanent() const {
        assert(n == m);
        assert(n < 63);
        std::vector<T> row(n, OperatorProperty::zero());  // 各行の、S に入っている列の和
        T res = OperatorProperty::zero();
        long long size = 0;  // |S|
        for (long long k = 1; k < (1LL << n); k++) {
            // グレイコードで k - 1 番目から k 番目に移るとき、k の最下位ビットの位置の列が出入りする
            long long j = 0;
            while (!(k >> j & 1)) j++;
            bool added = (k ^ (k >> 1)) >> j & 1;
            size += added ? 1 : -1;
            T prod = OperatorProperty::one();
            for (long long i = 0; i < n; i++) {
                row[i] = OperatorProperty::add(row[i], added ? this->v[i][j] : OperatorProperty::neg(this->v[i][j]));
                prod = OperatorProperty::mul(prod, row[i]);
            }
            res = OperatorProperty::add(res, (n - size) % 2 == 0 ? prod : OperatorProperty::neg(prod));
        }
        // S が空のときの項は、n == 0 なら 1、そうでなければ 0
        if (n == 0) res = OperatorProperty::one();
        return res;
    }

    /**
     * @brief 逆行列
     * @return 逆行列
     * @note det と同じく、要素は体 (modint や浮動小数点型など) であること
     */
    [[nodiscard]]
    Matrix inv() const {
        assert(n == m);
        if (n == 0) return Matrix(n, n);
        if constexpr (std::is_same_v<OperatorProperty, matrix::OperatorPropertyDefault<T>>) {
            std::vector A(n, std::valarray(T(0), n + n));
            for (long long i = 0; i < n; i++) {
                for (long long j = 0; j < n; j++) A[i][j] = this->v[i][j];
                A[i][n + i] = T(1);
            }
            [[maybe_unused]] T d = forward_elimination(A);
            assert(d != T(0));
            for (long long i = n - 1; i >= 0; i--) {
                for (long long j = i - 1; j >= 0; j--) {
                    A[j] -= A[i] * A[j][i];
                }
            }
            Matrix ret(n, n);
            for (long long i = 0; i < n; i++)
                for (long long j = 0; j < n; j++) ret[i][j] = A[i][n + j];
            return ret;
        } else {
            std::vector A(n, std::vector<T>(n + n, OperatorProperty::zero()));
            for (long long i = 0; i < n; i++) {
                for (long long j = 0; j < n; j++) A[i][j] = this->v[i][j];
                A[i][n + i] = OperatorProperty::one();
            }
            [[maybe_unused]] T d = forward_elimination(A);
            assert(d != OperatorProperty::zero());
            for (long long i = n - 1; i >= 0; i--) {
                for (long long j = i - 1; j >= 0; j--) {
                    for (long long k = n; k < n + n; k++) {
                        A[j][k] = OperatorProperty::add(A[j][k], OperatorProperty::neg(OperatorProperty::mul(A[i][k], A[j][i])));
                    }
                }
            }
            Matrix ret(n, n);
            for (long long i = 0; i < n; i++)
                for (long long j = 0; j < n; j++) ret[i][j] = A[i][n + j];
            return ret;
        }
    }

    /**
     * @brief 連立一次方程式 Ax = b の解
     * 解全体は x + c_0 basis[0] + c_1 basis[1] + ... (c_i は任意)
     */
    struct Solution {
        std::vector<T> x;                   //!< 解の 1 つ (特殊解)
        std::vector<std::vector<T>> basis;  //!< Ax = 0 の解空間の基底
    };

    /**
     * @brief 簡約化した行列 (行基本変形で、各行の主成分を 1 にし、主成分のある列の他の成分を 0 にしたもの) と、各行の主成分の列
     * 主成分の列の個数がランク。0 だけの行は下にまとまる
     * @note 割り算を使うので、要素は体 (modint など) であること。浮動小数点型は誤差を考えないので使えない
     */
    [[nodiscard]]
    std::pair<Matrix, std::vector<long long>> reduced_row_echelon() const {
        Matrix A = *this;
        std::vector<long long> pivots = reduce(A.v, m);
        return {A, pivots};
    }
    /**
     * @brief ランク O(nm min(n, m))
     */
    [[nodiscard]]
    long long rank() const {
        auto A = v;
        return reduce(A, m).size();
    }
    /**
     * @brief Ax = 0 の解空間の基底 (長さ m のベクトルを m - rank 個) O(nm min(n, m))
     */
    [[nodiscard]]
    std::vector<std::vector<T>> kernel() const {
        auto A = v;
        std::vector<long long> pivots = reduce(A, m);
        return kernel_from_reduced(A, pivots);
    }
    /**
     * @brief 連立一次方程式 Ax = b を解く。解がなければ std::nullopt O(nm min(n, m))
     */
    [[nodiscard]]
    std::optional<Solution> solve(const std::vector<T>& b) const {
        assert(static_cast<long long>(b.size()) == n);
        // 拡大係数行列 [A | b] を、A の列だけで簡約化する
        std::vector<std::vector<T>> A(n);
        for (long long i = 0; i < n; i++) {
            A[i] = v[i];
            A[i].push_back(b[i]);
        }
        std::vector<long long> pivots = reduce(A, m);
        const long long r = pivots.size();
        // 左側が 0 になった行の右辺が 0 でなければ解なし
        for (long long i = r; i < n; i++) {
            if (A[i][m] != OperatorProperty::zero()) return std::nullopt;
        }
        Solution sol;
        sol.x.assign(m, OperatorProperty::zero());
        for (long long i = 0; i < r; i++) sol.x[pivots[i]] = A[i][m];
        sol.basis = kernel_from_reduced(A, pivots);
        return sol;
    }

   private:
    // A の左から cols 列を使って簡約化し、各行の主成分の列を返す (列が cols より多い部分も一緒に変形する)
    static std::vector<long long> reduce(std::vector<std::vector<T>>& A, long long cols) {
        const long long rows = A.size();
        std::vector<long long> pivots;
        for (long long c = 0, r = 0; c < cols && r < rows; c++) {
            long long p = r;
            while (p < rows && A[p][c] == OperatorProperty::zero()) p++;
            if (p == rows) continue;
            std::swap(A[r], A[p]);
            const long long w = A[r].size();
            T scale = OperatorProperty::inv(A[r][c]);
            for (long long k = c; k < w; k++) A[r][k] = OperatorProperty::mul(A[r][k], scale);
            for (long long i = 0; i < rows; i++) {
                if (i == r || A[i][c] == OperatorProperty::zero()) continue;
                T f = A[i][c];
                for (long long k = c; k < w; k++) {
                    A[i][k] = OperatorProperty::add(A[i][k], OperatorProperty::neg(OperatorProperty::mul(f, A[r][k])));
                }
            }
            pivots.push_back(c);
            r++;
        }
        return pivots;
    }
    // 簡約化した A (左から m 列) から、Ax = 0 の解空間の基底を作る
    std::vector<std::vector<T>> kernel_from_reduced(const std::vector<std::vector<T>>& A, const std::vector<long long>& pivots) const {
        std::vector<bool> is_pivot(m, false);
        for (long long c : pivots) is_pivot[c] = true;
        std::vector<std::vector<T>> res;
        // 主成分のない列 (自由変数) f ごとに、x_f = 1、他の自由変数は 0 とした解
        for (long long f = 0; f < m; f++) {
            if (is_pivot[f]) continue;
            std::vector<T> x(m, OperatorProperty::zero());
            x[f] = OperatorProperty::one();
            for (long long i = 0; i < static_cast<long long>(pivots.size()); i++) x[pivots[i]] = OperatorProperty::neg(A[i][f]);
            res.push_back(x);
        }
        return res;
    }

   private:
    /**
     * @brief ガウスの消去法の前進消去を行って上三角行列を作り、行列式を返す
     * @param A 行列
     * @return 行列式
     * @note
     * rank(A) < nの場合は0を返す
     * 正方行列ではない場合、0以外が残る左からn個の列を使って行列式を計算する
     */
    T forward_elimination(std::vector<std::valarray<T>>& A) const {
        std::vector<long long> pivot_col(n, 0);
        T d(1);
        for (long long i = 0; i < n; i++) {
            if (i - 1 >= 0) pivot_col[i] = pivot_col[i - 1] + 1;
            while (pivot_col[i] < m) {
                long long pivot = i;
                if constexpr (std::is_floating_point_v<T>) {
                    for (long long j = i + 1; j < n; j++)
                        if (std::abs(A[j][pivot_col[i]]) > std::abs(A[pivot][pivot_col[i]])) pivot = j;
                    if (std::abs(A[pivot][pivot_col[i]]) < 1e-9) {
                        pivot_col[i]++;
                        continue;
                    }
                } else {
                    while (pivot < n && A[pivot][pivot_col[i]] == T(0)) pivot++;
                    if (pivot == n) {
                        pivot_col[i]++;
                        continue;
                    }
                }
                if (pivot != i) {
                    std::swap(A[i], A[pivot]);
                    d *= -T(1);
                }
                break;
            }
            if (pivot_col[i] == m) return T(0);
            T scale = A[i][pivot_col[i]];
            d *= scale;
            A[i] /= scale;
            for (long long j = i + 1; j < n; j++) A[j] -= A[i] * A[j][pivot_col[i]];
        }
        return d;
    }

    /**
     * @brief ガウスの消去法の前進消去を行って上三角行列を作り、行列式を返す
     * @param A 行列
     * @return 行列式
     * @note
     * rank(A) < nの場合は0を返す
     * 正方行列ではない場合、0以外が残る左からn個の列を使って行列式を計算する
     */
    T forward_elimination(std::vector<std::vector<T>>& A) const {
        std::vector<long long> pivot_col(n, 0);
        T d = OperatorProperty::one();
        for (long long i = 0; i < n; i++) {
            if (i - 1 >= 0) pivot_col[i] = pivot_col[i - 1] + 1;
            while (pivot_col[i] < m) {
                long long pivot = i;
                while (pivot < n && A[pivot][pivot_col[i]] == OperatorProperty::zero()) pivot++;
                if (pivot == n) {
                    pivot_col[i]++;
                    continue;
                }
                if (pivot != i) {
                    std::swap(A[i], A[pivot]);
                    d = OperatorProperty::neg(d);
                }
                break;
            }
            if (pivot_col[i] == m) return OperatorProperty::zero();
            const long long w = A[i].size();
            T scale = A[i][pivot_col[i]];
            d = OperatorProperty::mul(d, scale);
            for (long long j = pivot_col[i]; j < w; j++) A[i][j] = OperatorProperty::mul(A[i][j], OperatorProperty::inv(scale));
            for (long long j = i + 1; j < n; j++) {
                T scale = A[j][pivot_col[i]];
                for (long long k = pivot_col[i]; k < w; k++) {
                    A[j][k] = OperatorProperty::add(A[j][k], OperatorProperty::neg(OperatorProperty::mul(A[i][k], scale)));
                }
            }
        }
        return d;
    }
};

/**
 * @brief 行のリストから作るときの推論ガイド。要素の型は各行の value_type にする
 *
 * Matrix(std::vector<std::vector<int>>) は Matrix<int>、Matrix(std::vector<std::string>) は Matrix<char>
 */
template <typename Iterable>
Matrix(const std::vector<Iterable>&) -> Matrix<typename Iterable::value_type>;
