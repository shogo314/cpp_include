#pragma once

/**
 * @file sortable-segtree.hpp
 * @brief 区間ソートのできるセグメント木
 */

#include <cassert>
#include <utility>
#include <vector>

#include "fast-set.hpp"
#include "segtree.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/data_structures/sortable-segtree.hpp

/**
 * @brief 各位置にキーと値を持つ列で、1 点の変更・区間の積・区間をキーで並べ替える操作を行う
 *
 * 並べ替えた区間ごとに、その要素をキーの二分トライ (動的セグメント木) で持ち、昇順と降順の積を保つ。
 * 区間の先頭の位置に「区間の向きに合わせた積」を置いたセグメント木で区間の積を求める。
 * 並べ替えはトライの併合、区間の境目はトライを個数で分けることで作る
 * 計算量はならし O((N + Q) log K) (K はキーの上限)。メモリは O((N + Q) log K)
 * 例:
 * SortableSegTree<monoid::Sum<long long>> st(keys, values, 1LL << 30);
 * st.sort(l, r);         // [l, r) をキーの昇順に並べ替える
 * st.sort(l, r, true);   // 降順
 * st.prod(l, r);
 *
 * @tparam M モノイド (積の順番は列の順番)
 */
template <typename M>
class SortableSegTree {
    using S = typename M::S;
    struct Node {
        int child[2] = {0, 0};
        int cnt = 0;
        S asc, desc;  // キーの昇順・降順に並べたときの積
    };
    M m;
    long long n;
    int bits;
    std::vector<Node> nodes;  // nodes[0] は空のトライ
    std::vector<int> root;    // root[s]: s から始まる区間のトライ
    std::vector<bool> rev;    // rev[s]: s から始まる区間が降順か
    FastSet starts;           // 区間の先頭
    SegTree<M> seg;

    int new_node() {
        nodes.push_back(Node{{0, 0}, 0, m.e(), m.e()});
        return static_cast<int>(nodes.size()) - 1;
    }
    void pull(int t) {
        const Node &a = nodes[nodes[t].child[0]], &b = nodes[nodes[t].child[1]];
        nodes[t].cnt = a.cnt + b.cnt;
        nodes[t].asc = m.op(a.asc, b.asc);
        nodes[t].desc = m.op(b.desc, a.desc);
    }
    // キー k、値 x だけを持つトライ
    int make(long long k, const S& x) {
        assert(0 <= k && k < (1LL << bits));
        std::vector<int> path = {new_node()};
        for (int d = bits - 1; d >= 0; d--) {
            const int c = new_node();
            nodes[path.back()].child[k >> d & 1] = c;
            path.push_back(c);
        }
        nodes[path.back()].cnt = 1, nodes[path.back()].asc = nodes[path.back()].desc = x;
        for (int i = static_cast<int>(path.size()) - 2; i >= 0; i--) pull(path[i]);
        return path[0];
    }
    // 2 つのトライを併合する (キーは重ならないこと)
    int merge(int a, int b, int d) {
        if (a == 0) return b;
        if (b == 0) return a;
        assert(d >= 0 && "SortableSegTree: キーが重なっている");
        for (int c = 0; c < 2; c++) {
            int x = merge(nodes[a].child[c], nodes[b].child[c], d - 1);
            nodes[a].child[c] = x;
        }
        pull(a);
        return a;
    }
    // キーの小さい方から k 個と残りに分ける
    std::pair<int, int> split(int t, int k, int d) {
        if (t == 0 || k == 0) return {0, t};
        if (k >= nodes[t].cnt) return {t, 0};
        // ここに来るなら cnt >= 2 なので葉ではない
        int s = new_node();
        const int lc = nodes[t].child[0], left_cnt = nodes[lc].cnt;
        if (k <= left_cnt) {
            auto [a, b] = split(lc, k, d - 1);
            nodes[t].child[0] = a;
            nodes[s].child[0] = b, nodes[s].child[1] = nodes[t].child[1];
            nodes[t].child[1] = 0;
        } else {
            auto [a, b] = split(nodes[t].child[1], k - left_cnt, d - 1);
            nodes[t].child[1] = a;
            nodes[s].child[1] = b;
        }
        pull(t), pull(s);
        return {t, s};
    }
    // 区間の積を向きに合わせて返す
    S value(int s) const { return rev[s] ? nodes[root[s]].desc : nodes[root[s]].asc; }
    // x が区間の先頭になるように分ける
    void cut(long long x) {
        if (x >= n || starts.contains(x)) return;
        const long long s = starts.prev(x);
        const int len = nodes[root[s]].cnt, k = x - s;
        int a, b;
        if (!rev[s]) {
            std::tie(a, b) = split(root[s], k, bits - 1);
        } else {
            // 降順なので、先頭の k 個はキーの大きい方の k 個
            std::tie(b, a) = split(root[s], len - k, bits - 1);
        }
        root[s] = a, root[x] = b, rev[x] = rev[s];
        starts.insert(x);
        seg.set(s, value(s)), seg.set(x, value(x));
    }

   public:
    /**
     * @brief キーと値の列から作る
     * @param keys キー (0 <= キー < key_limit、すべて異なること)
     * @param key_limit キーの上限 (1 点の変更で使うキーもこれ未満であること)
     */
    SortableSegTree(const std::vector<long long>& keys, const std::vector<S>& values, long long key_limit, M m = M())
        : m(m), n(keys.size()), bits(1), root(n), rev(n, false), starts(n), seg(std::vector<S>(values), m) {
        assert(keys.size() == values.size());
        while ((1LL << bits) < key_limit) bits++;
        nodes.reserve(n * (bits + 2) + 1);
        nodes.push_back(Node{{0, 0}, 0, this->m.e(), this->m.e()});
        for (long long i = 0; i < n; i++) root[i] = make(keys[i], values[i]), starts.insert(i);
    }

    /**
     * @brief 要素数
     */
    long long size() const { return n; }
    /**
     * @brief 位置 i のキーと値を変える ならし O(log K)
     */
    void set(long long i, long long k, const S& x) {
        assert(0 <= i && i < n);
        cut(i), cut(i + 1);
        root[i] = make(k, x), rev[i] = false;
        seg.set(i, x);
    }
    /**
     * @brief a[l] a[l+1] ... a[r-1] の積 ならし O(log K + log N)
     */
    S prod(long long l, long long r) {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return m.e();
        cut(l), cut(r);
        return seg.prod(l, r);
    }
    /**
     * @brief [l, r) をキーの昇順 (reverse なら降順) に並べ替える ならし O(log K + 区間の個数)
     */
    void sort(long long l, long long r, bool reverse = false) {
        assert(0 <= l && l <= r && r <= n);
        if (l == r) return;
        cut(l), cut(r);
        int t = 0;
        for (long long s = l; s != -1 && s < r; s = starts.next(s + 1)) {
            t = merge(t, root[s], bits - 1);
            if (s != l) starts.erase(s), seg.set(s, m.e()), root[s] = 0;
        }
        root[l] = t, rev[l] = reverse;
        seg.set(l, value(l));
    }
};
