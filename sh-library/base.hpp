#include <bits/stdc++.h>

#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

using namespace std;
using ll = long long;
using ull = unsigned long long;
using str = string;
using pl = pair<ll, ll>;
template <typename T>
using ml = map<ll, T>;
using mll = map<ll, ll>;
using sl = set<ll>;
using ld = long double;
using pd = pair<ld, ld>;
template <typename T>
using vec = vector<T>;
template <typename T>
using vv = vector<vector<T>>;
template <typename T>
using vvv = vector<vector<vector<T>>>;
template <typename T1, typename T2>
using vp = vector<pair<T1, T2>>;
using vl = vec<ll>;
using vvl = vv<ll>;
using vvvl = vvv<ll>;
using vs = vec<str>;
using vc = vec<char>;
using vi = vec<int>;
using vb = vec<bool>;
using vpl = vec<pl>;
using spl = set<pl>;
using vd = vec<ld>;
using vpd = vec<pd>;
template <typename T, int N>
using ary = array<T, N>;
template <int N>
using al = array<ll, N>;
template <int N1, int N2>
using aal = array<array<ll, N2>, N1>;
template <int N>
using val = vec<al<N>>;

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

#define LL(x) ll x;cin >> x;
#define VL(a,n) vl a(n);cin >> a;
#define AL(a,k) al<k> a;cin >> a;
#define VC(a,n) vc a(n);cin >> a;
#define VS(a,n) vs a(n);cin >> a;
#define STR(s) str s;cin >> s;
#define VPL(a,n) vpl a(n);cin >> a;
#define VAL(a,n,k) val<k> a(n);cin >> a;
#define VVL(a,n,m) vvl a(n,vl(m));cin >> a;
#define SL(a,n) sl a;{VL(b,n);a=sl(all(b));}

template <typename T>
using heap_max = priority_queue<T, vec<T>, less<T>>;
template <typename T>
using heap_min = priority_queue<T, vec<T>, greater<T>>;
