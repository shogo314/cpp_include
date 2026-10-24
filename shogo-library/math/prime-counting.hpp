#pragma once

/**
 * @file prime-counting.hpp
 * @brief 素数の個数・素数の和 (Lucy DP)
 */

#include <cassert>
#include <type_traits>
#include <vector>

#include "sqrt.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/math/prime-counting.hpp

/**
 * @brief すべての v = floor(n / k) について、v 以下の素数 p の g(p) の和を求めた表 (Lucy DP)
 *
 * g は完全乗法的 (g(ab) = g(a) g(b)) であること。prefix(v) は g(2) + g(3) + ... + g(v) (2 以上 v 以下の整数すべての和)
 * 例: 素数の個数なら g(p) = 1、prefix(v) = v - 1。素数の和なら g(p) = p、prefix(v) = v (v + 1) / 2 - 1
 * O(n^(3/4) / log n)、メモリ O(sqrt(n))
 * 例:
 * PrimeSumTable<__int128> t(n, [](long long v) { return (__int128)v * (v + 1) / 2 - 1; }, [](long long p) { return (__int128)p; });
 * t(n);      // n 以下の素数の和
 * t(n / 3);  // floor(n / 3) 以下の素数の和
 *
 * @tparam T 和の型 (long long、__int128、modint など)
 */
template <typename T>
class PrimeSumTable {
    long long n, sq;
    std::vector<long long> vals;  // floor(n / k) のとる値の昇順 (1, 2, ..., sq と、sq より大きい n / k)
    std::vector<T> sum;           // sum[i] は vals[i] 以下の素数 p の g(p) の和

    long long index(long long v) const { return v <= sq ? v - 1 : static_cast<long long>(vals.size()) - n / v; }

   public:
    /**
     * @brief 表を作る
     * @pre n >= 1
     */
    template <typename Prefix, typename G>
    PrimeSumTable(long long n, Prefix prefix, G g) : n(n), sq(sqrtll(n)) {
        assert(n >= 1);
        for (long long v = 1; v <= sq; v++) vals.push_back(v);
        for (long long k = n / (sq + 1); k >= 1; k--) vals.push_back(n / k);
        sum.resize(vals.size());
        // はじめは 2 以上 v 以下の整数すべての和。小さい素数 p から順に、最小素因数が p の合成数の分を引く
        for (size_t i = 0; i < vals.size(); i++) sum[i] = prefix(vals[i]);
        std::vector<bool> composite(sq + 1, false);
        for (long long p = 2; p <= sq; p++) {
            if (composite[p]) continue;
            for (long long q = p * p; q <= sq; q += p) composite[q] = true;
            const T gp = g(p), below = sum[p - 2];  // below は p - 1 以下の素数の和
            const long long p2 = p * p;
            // v >= p^2 の値を、大きい方から (floor(v / p) がまだ更新されていないうちに) 更新する
            for (long long i = static_cast<long long>(vals.size()) - 1; i >= 0 && vals[i] >= p2; i--) {
                sum[i] -= gp * (sum[index(vals[i] / p)] - below);
            }
        }
    }

    /**
     * @brief v 以下の素数 p の g(p) の和 O(1)
     * @pre v = floor(n / k) となる k >= 1 があること (v = 0 も可)
     */
    T operator()(long long v) const {
        assert(0 <= v && v <= n);
        if (v == 0) return T(0);
        long long i = index(v);
        assert(vals[i] == v && "PrimeSumTable: v は floor(n / k) の形であること");
        return sum[i];
    }
    /**
     * @brief floor(n / k) のとる値の昇順
     */
    const std::vector<long long>& values() const { return vals; }
};

/**
 * @brief すべての v = floor(n / k) について、v 以下の素数の個数を求めた表 O(n^(3/4) / log n)
 * 例: auto t = prime_count_table(n); t(n / 2);
 */
inline PrimeSumTable<long long> prime_count_table(long long n) {
    return PrimeSumTable<long long>(n, [](long long v) { return v - 1; }, [](long long) { return 1LL; });
}

/**
 * @brief n 以下の素数の個数 π(n) O(n^(3/4) / log n)
 */
inline long long prime_count(long long n) {
    assert(n >= 0);
    if (n < 2) return 0;
    return prime_count_table(n)(n);
}

/**
 * @brief 乗法的関数 f の和 f(1) + f(2) + ... + f(n) (min_25 篩の後半) O(n^(3/4) / log n) 程度
 *
 * prime_sum(v) は v 以下の素数 p の f(p) の和。floor(n / k) のとる値 v ごとに 1 回ずつ呼んで表にする。PrimeSumTable を組み合わせて作る
 * f_pe(p, e) は f(p^e) (p は sqrt(n) 以下の素数、e >= 1)。f(1) = 1 とする
 * 例: オイラーの φ なら φ(p) = p - 1 なので、素数の和の表 s と個数の表 c から prime_sum(v) = s(v) - c(v)、f_pe(p, e) = p^e - p^(e-1)
 * @pre n >= 0
 */
template <typename PrimeSum, typename F>
auto multiplicative_sum(long long n, PrimeSum prime_sum, F f_pe) {
    using T = std::decay_t<decltype(prime_sum(n))>;
    assert(n >= 0);
    if (n == 0) return T(0);
    const long long sq = sqrtll(n);
    std::vector<long long> primes;
    std::vector<bool> composite(sq + 1, false);
    for (long long p = 2; p <= sq; p++) {
        if (composite[p]) continue;
        primes.push_back(p);
        for (long long q = p * p; q <= sq; q += p) composite[q] = true;
    }
    // below[j]: primes の先頭 j 個の f(p) の和
    std::vector<T> below(primes.size() + 1, T(0));
    for (size_t j = 0; j < primes.size(); j++) below[j + 1] = below[j] + f_pe(primes[j], 1);
    // prime_sum を floor(n / k) のとる値ごとに 1 回だけ呼んで表にする (v <= sq は v - 1 番目、それより大きい n / k は sq + k - 1 番目)
    std::vector<T> table(2 * sq + 1);
    for (long long v = 1; v <= sq; v++) table[v - 1] = prime_sum(v);
    for (long long k = 1; n / k > sq; k++) table[sq + k - 1] = prime_sum(n / k);
    auto at = [&](long long x) -> const T& { return x <= sq ? table[x - 1] : table[sq + n / x - 1]; };
    // 2 以上 x 以下で、最小素因数が primes[j] 以上の i の f(i) の和
    auto rec = [&](auto self, long long x, size_t j) -> T {
        // primes[j] 以上 (primes を使い切ったなら、primes のどれよりも大きい) の素数が x 以下にない
        if (j < primes.size() ? primes[j] > x : !primes.empty() && primes.back() >= x) return T(0);
        // 素数の分
        T res = at(x) - below[j];
        // 合成数の分: 最小素因数 p で p^e で割り切れて p^(e+1) で割り切れないものを、p^e と残りの積として数える
        for (size_t k = j; k < primes.size() && primes[k] * primes[k] <= x; k++) {
            const long long p = primes[k];
            if (p > x / (p * p)) {
                // p^3 > x なら e = 1 だけで、残り x / p (< p^2) は p より大きい素数か 1 なので、再帰せずに求める
                res += f_pe(p, 1) * (at(x / p) - below[k + 1]) + f_pe(p, 2);
                continue;
            }
            long long pe = p;
            for (long long e = 1; pe <= x / p; e++, pe *= p) {
                res += f_pe(p, e) * self(self, x / pe, k + 1) + f_pe(p, e + 1);
            }
        }
        return res;
    };
    return rec(rec, n, 0) + T(1);
}
