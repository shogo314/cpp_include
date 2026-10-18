#pragma once
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/base/type_alias.hpp

/**
 * @file type_alias.hpp
 * @brief 型の別名
 */

#include <array>
#include <cstddef>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

using ll = long long;
using ull = unsigned long long;
using ld = long double;

template <typename T>
using vec = std::vector<T>;
template <typename T, std::size_t N>
using ary = std::array<T, N>;
using str = std::string;
using std::deque;
using std::list;
using std::map;
using std::multimap;
using std::multiset;
using std::pair;
using std::set;

using pl = pair<ll, ll>;
using pd = pair<ld, ld>;

template <typename T>
using vv = vec<vec<T>>;
template <typename T>
using vvv = vec<vec<vec<T>>>;
using vl = vec<ll>;
using vvl = vv<ll>;
using vvvl = vvv<ll>;
using vs = vec<str>;
using vc = vec<char>;
using vi = vec<int>;
using vb = vec<bool>;

template <typename T1, typename T2>
using vp = vec<pair<T1, T2>>;
using vpl = vec<pl>;
using vvpl = vv<pl>;
using vd = vec<ld>;
using vpd = vec<pd>;

template <std::size_t N>
using al = ary<ll, N>;
template <std::size_t N1, std::size_t N2>
using aal = ary<ary<ll, N2>, N1>;
template <std::size_t N>
using val = vec<al<N>>;
template <std::size_t N>
using avl = ary<vl, N>;

template <typename T>
using ml = std::map<ll, T>;
using mll = std::map<ll, ll>;
using sl = std::set<ll>;
using spl = set<pl>;
template <std::size_t N>
using sal = set<al<N>>;
template <std::size_t N>
using asl = ary<sl, N>;

template <typename T>
using heap_max = std::priority_queue<T, std::vector<T>, std::less<T>>;
template <typename T>
using heap_min = std::priority_queue<T, std::vector<T>, std::greater<T>>;
