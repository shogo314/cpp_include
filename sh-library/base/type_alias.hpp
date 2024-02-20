#pragma once

#include <bits/stdc++.h>
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
using std::list;

template <typename T>
using heap_max = std::priority_queue<T, std::vector<T>, std::less<T>>;
template <typename T>
using heap_min = std::priority_queue<T, std::vector<T>, std::greater<T>>;
