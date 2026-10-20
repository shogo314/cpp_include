#pragma once
// https://github.com/shogo314/shogo-library/blob/93de89301cdeac655ce53761092d9556752fd185/src/string/suffix-automaton.hpp

/**
 * @file suffix-automaton.hpp
 * @brief Suffix Automaton
 */

#include <array>
#include <cassert>
#include <vector>

/**
 * @brief Suffix Automaton (文字列のすべての部分文字列を受理する最小の決定性オートマトン)
 *
 * 状態は O(n) 個 (2n - 1 個以下) で、各状態は endpos (出現する終わりの位置の集合) が同じ部分文字列の集まりを表す
 * 文字 c は c - Base として 0 以上 K 未満の番号に直す。遷移は大きさ K の配列で持つ。初期状態は 0 (空文字列)
 * 例:
 * SuffixAutomaton<26, 'a'> sa("abcbc");
 * sa.count_distinct_substrings();  // 12
 * sa.count_occurrences("bc");      // 2
 *
 * @tparam K 文字の種類数
 * @tparam Base 最小の文字
 */
template <int K = 26, int Base = 'a'>
class SuffixAutomaton {
    struct Node {
        std::array<int, K> next;
        int len;      // この状態の最長の部分文字列の長さ
        int link;     // suffix link (初期状態は -1)
        bool cloned;  // 分割のために複製した状態か
    };
    // メモリを抑えるため、状態の中身は int で持つ。外へ返すときに long long にする
    std::vector<Node> nodes;
    int last = 0;  // 文字列全体に対応する状態
    long long n = 0;
    std::vector<long long> occ;  // 各状態の endpos の大きさ (extend したら作り直す)
    bool occ_valid = false;

    int add_node(int len, int link, bool cloned) {
        Node node;
        node.next.fill(-1);
        node.len = len, node.link = link, node.cloned = cloned;
        nodes.push_back(node);
        return nodes.size() - 1;
    }
    static int index(int c) {
        assert(0 <= c - Base && c - Base < K);
        return c - Base;
    }
    template <typename R>
    int find_node(const R& t) const {
        int v = 0;
        for (auto c : t) {
            v = nodes[v].next[index(c)];
            if (v == -1) return -1;
        }
        return v;
    }
    void build_occ() {
        if (occ_valid) return;
        // 複製でない状態はちょうど 1 つの終わりの位置を持ち、suffix link の木の子孫の位置をまとめる
        occ.assign(nodes.size(), 0);
        for (size_t v = 1; v < nodes.size(); v++) occ[v] = nodes[v].cloned ? 0 : 1;
        // len の降順に、suffix link の先へ足す (計数ソート)
        std::vector<int> cnt(n + 1, 0), order(nodes.size());
        for (const auto& node : nodes) cnt[node.len]++;
        for (long long i = 1; i <= n; i++) cnt[i] += cnt[i - 1];
        for (int v = nodes.size() - 1; v >= 0; v--) order[--cnt[nodes[v].len]] = v;
        for (int i = nodes.size() - 1; i > 0; i--) {
            int v = order[i];
            occ[nodes[v].link] += occ[v];
        }
        occ_valid = true;
    }

   public:
    SuffixAutomaton() { add_node(0, -1, false); }
    /**
     * @brief 列 s の Suffix Automaton O(|s| K)
     * @param s std::string や std::vector<int> など、要素を整数に変換できる列
     */
    template <typename R>
    explicit SuffixAutomaton(const R& s) : SuffixAutomaton() {
        for (auto c : s) extend(c);
    }

    /**
     * @brief 末尾に文字 c を追加する ならし O(K)
     */
    void extend(int c) {
        int i = index(c);
        int cur = add_node(nodes[last].len + 1, -1, false);
        int p = last;
        while (p != -1 && nodes[p].next[i] == -1) {
            nodes[p].next[i] = cur;
            p = nodes[p].link;
        }
        if (p == -1) {
            nodes[cur].link = 0;
        } else {
            int q = nodes[p].next[i];
            if (nodes[p].len + 1 == nodes[q].len) {
                nodes[cur].link = q;
            } else {
                // q を、長さ len(p) + 1 までの部分 (複製) と残りに分ける
                int clone = add_node(nodes[p].len + 1, nodes[q].link, true);
                nodes[clone].next = nodes[q].next;
                while (p != -1 && nodes[p].next[i] == q) {
                    nodes[p].next[i] = clone;
                    p = nodes[p].link;
                }
                nodes[q].link = nodes[cur].link = clone;
            }
        }
        last = cur;
        n++;
        occ_valid = false;
    }

    /**
     * @brief 状態の個数 (初期状態を含む)
     */
    long long size() const { return nodes.size(); }
    /**
     * @brief 追加した文字列の長さ
     */
    long long length() const { return n; }
    /**
     * @brief 文字列全体に対応する状態
     */
    long long last_state() const { return last; }
    /**
     * @brief 状態 v の最長の部分文字列の長さ
     */
    long long len(long long v) const { return nodes[v].len; }
    /**
     * @brief 状態 v の suffix link (v の最長の部分文字列の接尾辞で、endpos が異なる最長のものの状態。初期状態なら -1)
     * 状態 v が表す部分文字列の長さは len(link(v)) + 1 以上 len(v) 以下
     */
    long long link(long long v) const { return nodes[v].link; }
    /**
     * @brief 状態 v から文字 c で進んだ状態 (なければ -1)
     */
    long long next(long long v, int c) const { return nodes[v].next[index(c)]; }

    /**
     * @brief 異なる部分文字列の個数 (空文字列を除く) O(状態数)
     */
    long long count_distinct_substrings() const {
        long long res = 0;
        for (size_t v = 1; v < nodes.size(); v++) res += nodes[v].len - nodes[nodes[v].link].len;
        return res;
    }
    /**
     * @brief t に対応する状態 (t が部分文字列でなければ -1) O(|t|)
     */
    template <typename R>
    long long find(const R& t) const { return find_node(t); }
    /**
     * @brief t が部分文字列か (空文字列は部分文字列) O(|t|)
     */
    template <typename R>
    bool contains(const R& t) const { return find_node(t) != -1; }
    /**
     * @brief 状態 v の部分文字列が出現する回数 (endpos の大きさ。初期状態なら length() + 1)
     * 最初に呼んだとき (と extend の後) に全状態の値を O(状態数) で求める
     */
    long long occurrences(long long v) {
        build_occ();
        return v == 0 ? n + 1 : occ[v];
    }
    /**
     * @brief t が部分文字列として出現する回数 (重なってもよい。空文字列なら length() + 1) O(|t|)
     */
    template <typename R>
    long long count_occurrences(const R& t) {
        int v = find_node(t);
        return v == -1 ? 0 : occurrences(v);
    }
    /**
     * @brief t の各位置 i について、t[0, i] の接尾辞のうち部分文字列であるものの最長の長さ O(|t|)
     * 最大値が t との最長共通部分文字列の長さになる
     * 範囲外の文字 (c - Base が 0 以上 K 未満でない) は、どの部分文字列にも含まれない文字として扱う
     */
    template <typename R>
    std::vector<long long> match_lengths(const R& t) const {
        std::vector<long long> res;
        // 今の位置で終わり、部分文字列である最長の文字列を、状態 v と長さ l で持つ
        int v = 0;
        long long l = 0;
        for (auto c : t) {
            const int k = static_cast<int>(c) - Base;
            if (k < 0 || K <= k) {
                v = 0, l = 0;
            } else {
                // c で進めない間は、前を削って endpos が増える短い接尾辞の状態に戻る
                // (初期状態まで戻ったときは l = 0 になっていて、c で進めなければそのまま)
                while (v != 0 && nodes[v].next[k] == -1) v = nodes[v].link, l = nodes[v].len;
                if (nodes[v].next[k] != -1) v = nodes[v].next[k], l++;
            }
            res.push_back(l);
        }
        return res;
    }
};
