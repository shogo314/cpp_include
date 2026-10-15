#pragma once
// https://github.com/shogo314/shogo-library/blob/24638d870d2ac03d01e1498411ab8aa477ea366f/src/data_structures/wavelet-matrix.hpp

/**
 * @file wavelet-matrix.hpp
 * @brief Wavelet Matrix
 */

#include <algorithm>
#include <cassert>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace internal_wavelet_matrix {
/**
 * @brief 0 と 1 の列で、ある位置より前の 1 の個数を O(1) で求める
 */
class BitVector {
    std::vector<unsigned long long> bits;
    std::vector<int> count;  // count[i] は i * 64 ビット目より前の 1 の個数

   public:
    BitVector() = default;
    explicit BitVector(int n) : bits(n / 64 + 1), count(n / 64 + 2) {}
    void set(int i) { bits[i >> 6] |= 1ULL << (i & 63); }
    void build() {
        for (int i = 0; i < int(bits.size()); i++) count[i + 1] = count[i] + __builtin_popcountll(bits[i]);
    }
    bool get(int i) const { return bits[i >> 6] >> (i & 63) & 1; }
    // [0, i) の 1 の個数
    int rank1(int i) const { return count[i >> 6] + __builtin_popcountll(bits[i >> 6] & ((1ULL << (i & 63)) - 1)); }
    // [0, i) の 0 の個数
    int rank0(int i) const { return i - rank1(i); }
};
}  // namespace internal_wavelet_matrix

/**
 * @brief Wavelet Matrix
 *
 * 静的な列に対して、区間の k 番目に小さい値や、区間内で値が [lower, upper) にある要素の個数を O(log σ) で求める
 * 値は座標圧縮して持つので、負の値や大きい値もそのまま使える (σ は値の種類数)
 * 例:
 * WaveletMatrix wm(std::vector<long long>{3, 1, 4, 1, 5});
 * wm.kth_smallest(1, 4, 0);  // 1 ([1, 4, 1] の 0 番目)
 * wm.count(0, 5, 2, 5);      // 2 (3 と 4)
 *
 * @tparam T 値の型 (< で比較できること)
 */
template <typename T>
class WaveletMatrix {
   protected:
    int n, height;
    std::vector<T> vals;  // 座標圧縮した値 (昇順)
    // bv[d] は上から d 番目の段 (ビット height - 1 - d) の列、mid[d] はその段の 0 の個数
    std::vector<internal_wavelet_matrix::BitVector> bv;
    std::vector<int> mid;

    // v 未満の値の圧縮後の番号
    int compressed_lower(const T& v) const { return std::lower_bound(vals.begin(), vals.end(), v) - vals.begin(); }
    // 区間 [l, r) を 1 段下の、0 側または 1 側の区間に移す
    std::pair<int, int> down(int d, int l, int r, bool b) const {
        if (b) return {mid[d] + bv[d].rank1(l), mid[d] + bv[d].rank1(r)};
        return {bv[d].rank0(l), bv[d].rank0(r)};
    }
    // [l, r) の、圧縮後の番号が c 未満の要素の個数
    int count_less(int l, int r, int c) const {
        int res = 0;
        for (int d = 0; d < height; d++) {
            bool b = c >> (height - 1 - d) & 1;
            if (b) res += bv[d].rank0(r) - bv[d].rank0(l);
            std::tie(l, r) = down(d, l, r, b);
        }
        return res;
    }
    // [l, r) の k 番目 (0-indexed) に小さい値の圧縮後の番号
    int kth_compressed(int l, int r, int k) const {
        int res = 0;
        for (int d = 0; d < height; d++) {
            int zeros = bv[d].rank0(r) - bv[d].rank0(l);
            bool b = k >= zeros;
            if (b) {
                k -= zeros;
                res |= 1 << (height - 1 - d);
            }
            std::tie(l, r) = down(d, l, r, b);
        }
        return res;
    }

   public:
    /**
     * @brief コンストラクタ
     *
     * @param v 列
     */
    explicit WaveletMatrix(const std::vector<T>& v = {}) : n(v.size()), vals(v) {
        std::sort(vals.begin(), vals.end());
        vals.erase(std::unique(vals.begin(), vals.end()), vals.end());
        // 圧縮後の番号 0, ..., vals.size() はすべて height ビットで表せるようにする
        height = 1;
        while ((1LL << height) <= (long long)vals.size()) height++;
        std::vector<int> cur(n), nxt(n);
        for (int i = 0; i < n; i++) cur[i] = compressed_lower(v[i]);
        bv.assign(height, internal_wavelet_matrix::BitVector(n));
        mid.assign(height, 0);
        for (int d = 0; d < height; d++) {
            int h = height - 1 - d;
            for (int i = 0; i < n; i++) {
                if (cur[i] >> h & 1) bv[d].set(i);
            }
            bv[d].build();
            mid[d] = bv[d].rank0(n);
            // 0 の要素を前に、1 の要素を後ろに、順序を保って並べる
            int zero = 0, one = mid[d];
            for (int i = 0; i < n; i++) {
                if (cur[i] >> h & 1)
                    nxt[one++] = cur[i];
                else
                    nxt[zero++] = cur[i];
            }
            std::swap(cur, nxt);
        }
    }

    /**
     * @brief 要素数を返す
     */
    long long size() const { return n; }

    /**
     * @brief k 番目の値を返す
     */
    T get(long long k) const {
        assert(0 <= k && k < n);
        int i = k, c = 0;
        for (int d = 0; d < height; d++) {
            bool b = bv[d].get(i);
            if (b) c |= 1 << (height - 1 - d);
            i = b ? mid[d] + bv[d].rank1(i) : bv[d].rank0(i);
        }
        return vals[c];
    }
    /**
     * @brief k 番目の値を返す
     */
    T operator[](long long k) const { return get(k); }

    /**
     * @brief [l, r) の要素を小さい順に並べたときの k 番目 (0-indexed) の値を返す
     */
    T kth_smallest(long long l, long long r, long long k) const {
        assert(0 <= l && l <= r && r <= n);
        assert(0 <= k && k < r - l);
        return vals[kth_compressed(l, r, k)];
    }
    /**
     * @brief [l, r) の要素を大きい順に並べたときの k 番目 (0-indexed) の値を返す
     */
    T kth_largest(long long l, long long r, long long k) const {
        assert(0 <= l && l <= r && r <= n);
        assert(0 <= k && k < r - l);
        return kth_smallest(l, r, r - l - 1 - k);
    }

    /**
     * @brief [l, r) の要素のうち、値が x のものの個数を返す
     */
    long long count(long long l, long long r, const T& x) const {
        assert(0 <= l && l <= r && r <= n);
        int c = compressed_lower(x);
        if (c == int(vals.size()) || x < vals[c]) return 0;
        return count_less(l, r, c + 1) - count_less(l, r, c);
    }
    /**
     * @brief [l, r) の要素のうち、値が [lower, upper) にあるものの個数を返す
     */
    long long count(long long l, long long r, const T& lower, const T& upper) const {
        assert(0 <= l && l <= r && r <= n);
        if (!(lower < upper)) return 0;
        return count_less(l, r, compressed_lower(upper)) - count_less(l, r, compressed_lower(lower));
    }

    /**
     * @brief [l, r) の要素のうち、upper 未満で最大の値を返す (なければ std::nullopt)
     */
    std::optional<T> prev_value(long long l, long long r, const T& upper) const {
        assert(0 <= l && l <= r && r <= n);
        int cnt = count_less(l, r, compressed_lower(upper));
        if (cnt == 0) return std::nullopt;
        return vals[kth_compressed(l, r, cnt - 1)];
    }
    /**
     * @brief [l, r) の要素のうち、lower 以上で最小の値を返す (なければ std::nullopt)
     */
    std::optional<T> next_value(long long l, long long r, const T& lower) const {
        assert(0 <= l && l <= r && r <= n);
        int cnt = count_less(l, r, compressed_lower(lower));
        if (cnt == r - l) return std::nullopt;
        return vals[kth_compressed(l, r, cnt)];
    }
};

template <typename T>
WaveletMatrix(std::vector<T>) -> WaveletMatrix<T>;

/**
 * @brief 重みの和も求められる Wavelet Matrix
 *
 * WaveletMatrix の機能に加えて、区間内で値が [lower, upper) にある要素の重みの和や、
 * 値の小さい方から k 個の重みの和を O(log σ) で求める。重みを省くと値そのものを重みにする
 * メモリは WaveletMatrix より (log σ + 1) * (n + 1) 個の W だけ多い
 * 例:
 * WeightedWaveletMatrix wm(std::vector<long long>{3, 1, 4, 1, 5});
 * wm.sum(0, 5, 2, 5);           // 7 (3 + 4)
 * wm.sum_smallest(0, 5, 3);     // 5 (1 + 1 + 3)
 *
 * @tparam T 値の型 (< で比較できること)
 * @tparam W 重みの型 (和を取る)。省略すると、T が整数なら long long、そうでなければ T
 */
template <typename T, typename W = std::conditional_t<std::is_integral_v<T>, long long, T>>
class WeightedWaveletMatrix : public WaveletMatrix<T> {
    using Base = WaveletMatrix<T>;
    using Base::bv;
    using Base::height;
    using Base::n;
    // pre[d][i] は、上から d 段目まで分けた後の列 (d = 0 なら元の列) の先頭 i 個の重みの和
    std::vector<std::vector<W>> pre;

    void build_sum(const std::vector<W>& w) {
        assert(int(w.size()) == n);
        std::vector<int> order(n), nxt(n);  // 各段の列の、元の列での位置
        for (int i = 0; i < n; i++) order[i] = i;
        pre.assign(height + 1, std::vector<W>(n + 1, W(0)));
        for (int d = 0;; d++) {
            for (int i = 0; i < n; i++) pre[d][i + 1] = pre[d][i] + w[order[i]];
            if (d == height) break;
            int zero = 0, one = this->mid[d];
            for (int i = 0; i < n; i++) {
                if (bv[d].get(i))
                    nxt[one++] = order[i];
                else
                    nxt[zero++] = order[i];
            }
            std::swap(order, nxt);
        }
    }
    // [l, r) の、圧縮後の番号が c 未満の要素の重みの和
    W sum_less(int l, int r, int c) const {
        W res(0);
        for (int d = 0; d < height; d++) {
            bool b = c >> (height - 1 - d) & 1;
            if (b) res += pre[d + 1][bv[d].rank0(r)] - pre[d + 1][bv[d].rank0(l)];
            std::tie(l, r) = this->down(d, l, r, b);
        }
        return res;
    }

   public:
    /**
     * @brief コンストラクタ (値そのものを重みにする)
     *
     * @param v 列
     */
    explicit WeightedWaveletMatrix(const std::vector<T>& v = {}) : Base(v) { build_sum(std::vector<W>(v.begin(), v.end())); }
    /**
     * @brief コンストラクタ
     *
     * @param v 列
     * @param w 各要素の重み
     */
    WeightedWaveletMatrix(const std::vector<T>& v, const std::vector<W>& w) : Base(v) { build_sum(w); }

    /**
     * @brief [l, r) の要素のうち、値が [lower, upper) にあるものの重みの和を返す
     */
    W sum(long long l, long long r, const T& lower, const T& upper) const {
        assert(0 <= l && l <= r && r <= n);
        if (!(lower < upper)) return W(0);
        return sum_less(l, r, this->compressed_lower(upper)) - sum_less(l, r, this->compressed_lower(lower));
    }
    /**
     * @brief [l, r) のすべての要素の重みの和を返す
     */
    W sum(long long l, long long r) const {
        assert(0 <= l && l <= r && r <= n);
        return pre[0][r] - pre[0][l];
    }
    /**
     * @brief [l, r) の要素を値の小さい順に並べたときの、先頭 k 個の重みの和を返す
     *
     * 同じ値の要素は、元の列で前にあるものから数える
     */
    W sum_smallest(long long l_, long long r_, long long k_) const {
        assert(0 <= l_ && l_ <= r_ && r_ <= n);
        assert(0 <= k_ && k_ <= r_ - l_);
        int l = l_, r = r_, k = k_;
        W res(0);
        for (int d = 0; d < height; d++) {
            int l0 = bv[d].rank0(l), r0 = bv[d].rank0(r);
            bool b = k >= r0 - l0;
            if (b) {
                res += pre[d + 1][r0] - pre[d + 1][l0];
                k -= r0 - l0;
            }
            std::tie(l, r) = this->down(d, l, r, b);
        }
        // 残りの k 個は同じ値で、最後の段の [l, r) の先頭に並んでいる
        return res + pre[height][l + k] - pre[height][l];
    }
    /**
     * @brief [l, r) の要素を値の大きい順に並べたときの、先頭 k 個の重みの和を返す
     *
     * 同じ値の要素は、元の列で後ろにあるものから数える
     */
    W sum_largest(long long l, long long r, long long k) const {
        assert(0 <= l && l <= r && r <= n);
        assert(0 <= k && k <= r - l);
        return sum(l, r) - sum_smallest(l, r, r - l - k);
    }
};

template <typename T>
WeightedWaveletMatrix(std::vector<T>) -> WeightedWaveletMatrix<T>;
template <typename T, typename W>
WeightedWaveletMatrix(std::vector<T>, std::vector<W>) -> WeightedWaveletMatrix<T, W>;
