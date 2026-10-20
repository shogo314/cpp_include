#pragma once

/**
 * @file prime-counting.hpp
 * @brief 素数の個数・素数の和 (Lucy DP)
 */

#include <cassert>
#include <vector>

#include "sqrt.hpp"
// https://github.com/shogo314/shogo-library/blob/6326ed0de9725c66d835297239e902b9eb0cdfc3/src/math/prime-counting.hpp

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
