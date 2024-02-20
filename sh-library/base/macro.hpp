#pragma once

#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#define all(obj) (obj).begin(), (obj).end()

#define reps(i, a, n) for (long long i = (a); i < (n); i++)
#define rep(i, n) reps(i, 0, (n))
#define rrep(i, n) reps(i, 1, (n) + 1)
#define repds(i, a, n) for (long long i = (n)-1; i >= (a); i--)
#define repd(i, n) repds(i, 0, (n))
#define rrepd(i, n) repds(i, 1, (n) + 1)
#define rep2(i, j, x, y) rep(i, x) rep(j, y)

#define IN(a, x) a x; std::cin >> x;
