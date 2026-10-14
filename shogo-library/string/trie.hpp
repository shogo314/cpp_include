#pragma once
// https://github.com/shogo314/shogo-library/blob/a2fefc7ec36d798ade24522d799381341eac20dd/src/string/trie.hpp

/**
 * @file trie.hpp
 * @brief Trie
 */

#include <array>
#include <cassert>
#include <vector>

/**
 * @brief Trie (文字列の集合を接頭辞の木で管理する)
 *
 * 文字 c は c - Base として 0 以上 K 未満の番号に直す。根はノード 0 (空文字列)
 * 同じ文字列を何度も追加でき、多重集合として扱う
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
        int prefix_cnt;  // このノードを通る (このノードの文字列を接頭辞に持つ) 文字列の個数
        int word_cnt;    // このノードで終わる文字列の個数
    };
    std::vector<Node> nodes;

    int add_node(int parent, int depth) {
        Node node;
        node.next.fill(-1);
        node.parent = parent;
        node.depth = depth;
        node.prefix_cnt = node.word_cnt = 0;
        nodes.push_back(node);
        return nodes.size() - 1;
    }
    static int index(int c) {
        assert(0 <= c - Base && c - Base < K);
        return c - Base;
    }

   public:
    Trie() { add_node(-1, 0); }

    /**
     * @brief ノードの個数 (根を含む)
     */
    int size() const { return nodes.size(); }

    /**
     * @brief 文字列 s を 1 個追加し、s に対応するノードを返す O(|s|)
     * @param s std::string や std::vector<int> など、要素を整数に変換できる列
     */
    template <typename R>
    int insert(const R& s) {
        int v = 0;
        nodes[v].prefix_cnt++;
        for (auto c : s) {
            int i = index(c);
            if (nodes[v].next[i] == -1) {
                int u = add_node(v, nodes[v].depth + 1);
                nodes[v].next[i] = u;
            }
            v = nodes[v].next[i];
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
        int v = find(s);
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
    int find(const R& s) const {
        int v = 0;
        for (auto c : s) {
            v = nodes[v].next[index(c)];
            if (v == -1) return -1;
        }
        return v;
    }

    /**
     * @brief 文字列 s の個数 O(|s|)
     */
    template <typename R>
    int count(const R& s) const {
        int v = find(s);
        return v == -1 ? 0 : nodes[v].word_cnt;
    }
    /**
     * @brief s を接頭辞に持つ文字列の個数 O(|s|)
     */
    template <typename R>
    int count_prefix(const R& s) const {
        int v = find(s);
        return v == -1 ? 0 : nodes[v].prefix_cnt;
    }
    /**
     * @brief 文字列の個数 (重複を含む)
     */
    int total() const { return nodes[0].prefix_cnt; }

    /**
     * @brief ノード v から文字 c で進んだノード (なければ -1)
     */
    int child(int v, int c) const { return nodes[v].next[index(c)]; }
    /**
     * @brief ノード v の親 (根なら -1)
     */
    int parent(int v) const { return nodes[v].parent; }
    /**
     * @brief ノード v の深さ (対応する文字列の長さ)
     */
    int depth(int v) const { return nodes[v].depth; }
    /**
     * @brief ノード v を通る (v の文字列を接頭辞に持つ) 文字列の個数
     */
    int count_prefix_at(int v) const { return nodes[v].prefix_cnt; }
    /**
     * @brief ノード v で終わる文字列の個数
     */
    int count_at(int v) const { return nodes[v].word_cnt; }

    /**
     * @brief s の接頭辞のうち、追加した文字列のどれかの接頭辞にもなっているものの最長の長さ O(|s|)
     * 追加した文字列 (取り除いたものは除く) のどれかとの最長共通接頭辞の長さの最大値になる
     */
    template <typename R>
    int longest_common_prefix(const R& s) const {
        int v = 0, len = 0;
        for (auto c : s) {
            int u = nodes[v].next[index(c)];
            if (u == -1 || nodes[u].prefix_cnt == 0) break;
            v = u;
            len++;
        }
        return len;
    }
};
