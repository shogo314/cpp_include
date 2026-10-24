#pragma once
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/string/trie.hpp

/**
 * @file trie.hpp
 * @brief Trie
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @brief Trie (文字列の集合を接頭辞の木で管理する)
 *
 * 文字 c は c - Base として 0 以上 K 未満の番号に直す。根はノード 0 (空文字列)
 * 同じ文字列を何度も追加でき、多重集合として扱う
 * ノードは jump pointer を持ち、2 つのノードの最小共通祖先や文字列の辞書順の比較が O(log n) でできる
 * 例:
 * Trie<26, 'a'> trie;
 * trie.insert("abc");
 * trie.count_prefix("ab");  // 1
 *
 * @tparam K 文字の種類数
 * @tparam Base 最小の文字
 */
template <int K = 26, int Base = 'a'>
class Trie {
    struct Node {
        std::array<int, K> next;
        int parent, depth;
        int ch;          // 親から来た辺の文字の番号 (根は -1)
        int jump;        // 祖先への飛び先 (深さの差が 1, 1, 3, 1, 1, 3, 7, ... となる skew-binary の飛び方)
        int prefix_cnt;  // このノードを通る (このノードの文字列を接頭辞に持つ) 文字列の個数
        int word_cnt;    // このノードで終わる文字列の個数
    };
    // メモリを抑えるため、ノードの中身は int で持つ。外へ返すときに long long にする
    std::vector<Node> nodes;

    int add_node(int parent, int ch) {
        Node node;
        node.next.fill(-1);
        node.parent = parent;
        node.ch = ch;
        node.prefix_cnt = node.word_cnt = 0;
        if (parent == -1) {
            node.depth = 0;
            node.jump = 0;
        } else {
            node.depth = nodes[parent].depth + 1;
            // 親の飛び先 2 つの深さの差が等しければ、2 つをまとめて飛ぶ
            int j = nodes[parent].jump, jj = nodes[j].jump;
            node.jump = nodes[parent].depth - nodes[j].depth == nodes[j].depth - nodes[jj].depth ? jj : parent;
        }
        nodes.push_back(node);
        return nodes.size() - 1;
    }
    // v の子 i (なければ作る)
    int extend_node(int v, int i) {
        if (nodes[v].next[i] == -1) {
            int u = add_node(v, i);
            nodes[v].next[i] = u;
        }
        return nodes[v].next[i];
    }
    // v の深さ d の祖先
    int ancestor(int v, int d) const {
        while (nodes[v].depth > d) v = nodes[nodes[v].jump].depth >= d ? nodes[v].jump : nodes[v].parent;
        return v;
    }
    // 深さが等しく異なる u, v を、親が一致する直前まで持ち上げる (飛び先は深さだけで決まるので、そろって飛べる)
    std::pair<int, int> below_lca(int u, int v) const {
        while (nodes[u].parent != nodes[v].parent) {
            if (nodes[u].jump != nodes[v].jump) {
                u = nodes[u].jump, v = nodes[v].jump;
            } else {
                u = nodes[u].parent, v = nodes[v].parent;
            }
        }
        return {u, v};
    }
    // 文字列リテラル ("abc" は char[4]) を末尾の '\0' まで読まないよう、文字列は string_view にする
    template <typename R>
    static decltype(auto) as_range(const R& s) {
        if constexpr (std::is_convertible_v<const R&, std::string_view>) {
            return std::string_view(s);
        } else {
            return (s);
        }
    }
    static int index(int c) {
        assert(0 <= c - Base && c - Base < K);
        return c - Base;
    }
    // 文字列 s に対応するノード (なければ -1)
    template <typename R>
    int find_node(const R& s) const {
        int v = 0;
        for (auto c : as_range(s)) {
            v = nodes[v].next[index(c)];
            if (v == -1) return -1;
        }
        return v;
    }

   public:
    Trie() { add_node(-1, -1); }

    /**
     * @brief ノードの個数 (根を含む)
     */
    long long size() const { return nodes.size(); }

    /**
     * @brief 文字列 s を 1 個追加し、s に対応するノードを返す O(|s|)
     * @param s std::string や std::vector<int> など、要素を整数に変換できる列
     */
    template <typename R>
    long long insert(const R& s) {
        int v = 0;
        nodes[v].prefix_cnt++;
        for (auto c : as_range(s)) {
            v = extend_node(v, index(c));
            nodes[v].prefix_cnt++;
        }
        nodes[v].word_cnt++;
        return v;
    }

    /**
     * @brief 文字列 s を 1 個取り除く。s がなければ何もせず false を返す O(|s|)
     * ノードは削除しないので、ノードの番号は変わらない
     */
    template <typename R>
    bool erase(const R& s) {
        int v = find_node(s);
        if (v == -1 || nodes[v].word_cnt == 0) return false;
        nodes[v].word_cnt--;
        for (; v != -1; v = nodes[v].parent) nodes[v].prefix_cnt--;
        return true;
    }

    /**
     * @brief 文字列 s に対応するノード (なければ -1) O(|s|)
     * s を接頭辞に持つ文字列を追加したことがあれば、取り除いた後でもノードは残る
     */
    template <typename R>
    long long find(const R& s) const { return find_node(s); }

    /**
     * @brief 文字列 s の個数 O(|s|)
     */
    template <typename R>
    long long count(const R& s) const {
        int v = find_node(s);
        return v == -1 ? 0 : nodes[v].word_cnt;
    }
    /**
     * @brief s を接頭辞に持つ文字列の個数 O(|s|)
     */
    template <typename R>
    long long count_prefix(const R& s) const {
        int v = find_node(s);
        return v == -1 ? 0 : nodes[v].prefix_cnt;
    }
    /**
     * @brief 文字列の個数 (重複を含む)
     */
    long long total() const { return nodes[0].prefix_cnt; }

    /**
     * @brief ノード v から文字 c で進んだノード (なければ -1)
     */
    long long child(long long v, int c) const { return nodes[v].next[index(c)]; }
    /**
     * @brief ノード v から文字 c で進んだノード。なければ作る (文字列の個数は変えない) O(K)
     */
    long long extend(long long v, int c) { return extend_node(v, index(c)); }
    /**
     * @brief ノード v の親 (根なら -1)
     */
    long long parent(long long v) const { return nodes[v].parent; }
    /**
     * @brief ノード v の深さ (対応する文字列の長さ)
     */
    long long depth(long long v) const { return nodes[v].depth; }
    /**
     * @brief ノード v を通る (v の文字列を接頭辞に持つ) 文字列の個数
     */
    long long count_prefix_at(long long v) const { return nodes[v].prefix_cnt; }
    /**
     * @brief ノード v で終わる文字列の個数
     */
    long long count_at(long long v) const { return nodes[v].word_cnt; }

    /**
     * @brief ノード u, v の最小共通祖先 (2 つの文字列の最長共通接頭辞のノード) O(log n)
     */
    long long lca(long long u, long long v) const {
        int d = std::min(nodes[u].depth, nodes[v].depth);
        int a = ancestor(u, d), b = ancestor(v, d);
        if (a == b) return a;
        return nodes[below_lca(a, b).first].parent;
    }
    /**
     * @brief ノード u の文字列が v の文字列より辞書順で小さいか O(log n)
     * 一方が他方の接頭辞なら、短いほうが小さい
     */
    bool less(long long u, long long v) const {
        int d = std::min(nodes[u].depth, nodes[v].depth);
        int a = ancestor(u, d), b = ancestor(v, d);
        if (a == b) return nodes[u].depth < nodes[v].depth;
        auto [x, y] = below_lca(a, b);
        return nodes[x].ch < nodes[y].ch;
    }

    /**
     * @brief s の接頭辞のうち、追加した文字列のどれかの接頭辞にもなっているものの最長の長さ O(|s|)
     * 追加した文字列 (取り除いたものは除く) のどれかとの最長共通接頭辞の長さの最大値になる
     */
    template <typename R>
    long long longest_common_prefix(const R& s) const {
        int v = 0;
        long long len = 0;
        for (auto c : as_range(s)) {
            int u = nodes[v].next[index(c)];
            if (u == -1 || nodes[u].prefix_cnt == 0) break;
            v = u;
            len++;
        }
        return len;
    }
};
