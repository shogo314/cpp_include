#include <bits/stdc++.h>

#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

using ll = long long;
using ull = unsigned long long;
using str = std::string;
using pl = std::pair<ll, ll>;
template <typename T>
using ml = std::map<ll, T>;
using mll = std::map<ll, ll>;
using sl = std::set<ll>;
using ld = long double;
using pd = std::pair<ld, ld>;
template <typename T>
using vec = std::vector<T>;
template <typename T>
using vv = vec<vec<T>>;
template <typename T>
using vvv = vec<vec<vec<T>>>;
template <typename T1, typename T2>
using vp = vec<std::pair<T1, T2>>;
using vl = vec<ll>;
using vvl = vv<ll>;
using vvvl = vvv<ll>;
using vs = vec<str>;
using vc = vec<char>;
using vi = vec<int>;
using vb = vec<bool>;
using vpl = vec<pl>;
using vvpl = vv<pl>;
using spl = std::set<pl>;
using vd = vec<ld>;
using vpd = vec<pd>;
template <typename T, int N>
using ary = std::array<T, N>;
template <int N>
using al = std::array<ll, N>;
template <int N1, int N2>
using aal = ary<ary<ll, N2>, N1>;
template <int N>
using val = vec<al<N>>;
template <int N>
using sal = set<al<N>>;
using std::deque;

#define all(obj) (obj).begin(), (obj).end()

#define reps(i, a, n) for (ll i = (a); i < ll(n); i++)
#define rep(i, n) reps(i, 0, (n))
#define rrep(i, n) reps(i, 1, (n) + 1)
#define repds(i, a, n) for (ll i = ((n) - 1); i >= (a); i--)
#define repd(i, n) repds(i, 0, (n))
#define rrepd(i, n) repds(i, 1, (n) + 1)
#define rep2(i, j, x, y) rep(i, x) rep(j, y)

template <typename T1, typename T2>
inline bool chmin(T1 &a, T2 b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}

template <typename T1, typename T2>
inline bool chmax(T1 &a, T2 b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}

// #define LL(x) ll x;cin >> x;
// #define VL(a,n) vl a(n);cin >> a;
// #define AL(a,k) al<k> a;cin >> a;
// #define VC(a,n) vc a(n);cin >> a;
// #define VS(a,n) vs a(n);cin >> a;
// #define STR(s) str s;cin >> s;
// #define VPL(a,n) vpl a(n);cin >> a;
// #define VAL(a,n,k) val<k> a(n);cin >> a;
// #define VVL(a,n,m) vvl a(n,vl(m));cin >> a;
// #define SL(a,n) sl a;{VL(b,n);a=sl(all(b));}
#define IN(a,x) a x; std::cin >> x;

template <typename T>
using heap_max = std::priority_queue<T, vec<T>, less<T>>;
template <typename T>
using heap_min = std::priority_queue<T, vec<T>, greater<T>>;
