#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/utility/two-pointers.hpp

/**
 * @file two-pointers.hpp
 * @brief 尺取り法
 */

#include <vector>

/**
 * @brief 尺取り法で、各 l について区間 [l, r) を右に伸ばせるだけ伸ばした r を求める
 *
 * can_add(l, r) は、いまの区間 [l, r) に r を加えてもよいかを返す。add(r) は r を加えたとき、erase(l) は l を取り除いたときに呼ばれる
 * [l, r) に r を加えてよいなら [l + 1, r) にも加えてよい (区間を左から縮めても条件が崩れない) こと
 * can_add(l, l) が偽なら、区間は空のまま r = l になる。空の区間からは何も取り除かない
 *
 * 円環の列は、1 周ずらした写しを後ろにつなげた長さ 2n の列に使い、l < n の分を見る
 *
 * @param n 列の長さ
 * @return 長さ n の列。l 番目は [l, r) を伸ばし終えたときの r
 */
template <class CanAdd, class Add, class Erase>
std::vector<long long> two_pointers(long long n, CanAdd can_add, Add add, Erase erase) {
    std::vector<long long> res(n);
    long long r = 0;
    for (long long l = 0; l < n; l++) {
        if (r < l) r = l;
        while (r < n && can_add(l, r)) add(r++);
        res[l] = r;
        if (l < r) erase(l);
    }
    return res;
}

/**
 * @brief 状態を持たない尺取り法 (can_add(l, r) が l と r だけで決まるとき)
 */
template <class CanAdd>
std::vector<long long> two_pointers(long long n, CanAdd can_add) {
    return two_pointers(n, can_add, [](long long) {}, [](long long) {});
}
