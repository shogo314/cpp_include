#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/dp/set-cover.hpp

/**
 * @file set-cover.hpp
 * @brief 集合を「使ってよい集合」で覆うときの最小個数 (包除原理、要素が 20 個程度まで)
 */

#include <cassert>
#include <vector>

/**
 * @brief {0, ..., n-1} を、feasible[T] が true の集合 T いくつかで (重なりを許して) 覆うときの最小個数 (覆えなければ -1) O(2^n n)
 * feasible の大きさは 2^n。f(S) を S に含まれる使ってよい集合の数とすると、全体を覆う k 個組の数は Σ_S (-1)^{n - |S|} f(S)^k。
 * これが 0 でない最小の k を求める。数は 2 つの素数の mod で数えるので、0 でないのに 0 と誤る確率がわずかにある
 */
inline long long min_set_cover(int n, const std::vector<bool>& feasible) {
    assert(0 <= n && n < 31);
    assert(static_cast<long long>(feasible.size()) == (1LL << n));
    if (n == 0) return 0;
    const int N = 1 << n;
    // f[S] = S に含まれる使ってよい集合の数 (部分集合についてのゼータ変換)。2^n 以下なので mod を取らずに持てる
    std::vector<unsigned> f(N);
    for (int S = 0; S < N; S++) f[S] = feasible[S];
    for (int i = 0; i < n; i++) {
        for (int S = 0; S < N; S++) {
            if (S >> i & 1) f[S] += f[S ^ (1 << i)];
        }
    }
    if (f[N - 1] == 0) return -1;
    static constexpr unsigned long long P[2] = {998244353, 1000000007};
    // pw[j][S] = f[S]^k mod P[j]
    std::vector<unsigned> pw[2] = {std::vector<unsigned>(N, 1), std::vector<unsigned>(N, 1)};
    // 覆えるなら、各要素を含む集合を 1 つずつ選べば n 個以下で覆える
    for (int k = 1; k <= n; k++) {
        bool nonzero = false;
        for (int j = 0; j < 2; j++) {
            unsigned long long sum = 0;
            for (int S = 0; S < N; S++) {
                pw[j][S] = static_cast<unsigned>(pw[j][S] * static_cast<unsigned long long>(f[S]) % P[j]);
                // n - |S| が奇数なら引く
                sum += (__builtin_parity(S) ^ (n & 1)) ? P[j] - pw[j][S] : pw[j][S];
            }
            nonzero |= sum % P[j] != 0;
        }
        if (nonzero) return k;
    }
    return -1;
}
