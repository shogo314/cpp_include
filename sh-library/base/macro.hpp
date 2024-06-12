#pragma once
#include "type_alias.hpp"

#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#define all(obj) (obj).begin(), (obj).end()

#define repts(i, a, n, t) for (long long i = (a); i < (n); i+=(t))
#define reps(i, a, n) for (long long i = (a); i < (n); i++)
#define rep(i, n) reps(i, 0, (n))
#define rrep(i, n) reps(i, 1, (n) + 1)
#define repds(i, a, n) for (long long i = (n)-1; i >= (a); i--)
#define repd(i, n) repds(i, 0, (n))
#define rrepd(i, n) repds(i, 1, (n) + 1)
#define rep2(i, j, x, y) rep(i, x) rep(j, y)

inline void scan(){}
template<class Head,class... Tail>
inline void scan(Head&head,Tail&... tail){std::cin>>head;scan(tail...);}
#define LL(...) ll __VA_ARGS__;scan(__VA_ARGS__)
#define STR(...) str __VA_ARGS__;scan(__VA_ARGS__)
#define IN(a, x) a x; std::cin >> x;
#define CHAR(x) char x; std::cin >> x;
#define VL(a,n) vl a(n); std::cin >> a;
#define AL(a,k) al<k> a; std::cin >> a;
#define AAL(a,n,m) aal<n,m> a; std::cin >> a;
#define VC(a,n) vc a(n); std::cin >> a;
#define VS(a,n) vs a(n); std::cin >> a;
#define VPL(a,n) vpl a(n); std::cin >> a;
#define VAL(a,n,k) val<k> a(n); std::cin >> a;
#define VVL(a,n,m) vvl a(n,vl(m)); std::cin >> a;
#define SL(a,n) sl a;{VL(b,n);a=sl(all(b));}

#define NO std::cout << "NO" << std::endl; return;
#define YES std::cout << "YES" << std::endl; return;
#define No std::cout << "No" << std::endl; return;
#define Yes std::cout << "Yes" << std::endl; return;
#define Takahashi std::cout << "Takahashi" << std::endl; return;
#define Aoki std::cout << "Aoki" << std::endl; return;
