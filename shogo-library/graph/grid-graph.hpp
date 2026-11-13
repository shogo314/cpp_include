#pragma once

/**
 * @file grid-graph.hpp
 * @brief 文字列盤面から作るグリッドグラフ
 */

#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "../utility/range.hpp"
#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/9a3850ecb2614059a50786efbc668c34530bbd74/src/graph/grid-graph.hpp

/**
 * @brief 壁以外のマスを頂点とする、4 近傍・重み 1 の無向グラフ
 * 元の盤面と座標対応を保持する。頂点番号は通行可能なマスに行優先で付ける。
 */
class GridGraph {
    std::vector<std::string> data;
    long long h, w;
    std::vector<long long> ids;
    std::vector<Grid::value_type> cells;
    Graph<long long> g;

    class Matches {
        const GridGraph* owner;
        char c;

       public:
        Matches(const GridGraph* owner, char c) : owner(owner), c(c) {}

        class iterator {
            const GridGraph* owner = nullptr;
            char c = 0;
            long long v = -1;

            void seek() {
                while (v < owner->size() && owner->label(v) != c) ++v;
                if (v == owner->size()) v = -1;
            }

           public:
            using value_type = long long;
            using difference_type = std::ptrdiff_t;
            using reference = value_type;
            using pointer = void;
            using iterator_category = std::input_iterator_tag;
            using iterator_concept = std::forward_iterator_tag;

            iterator() = default;
            iterator(const GridGraph* owner, char c) : owner(owner), c(c), v(0) { seek(); }
            value_type operator*() const { return v; }
            iterator& operator++() {
                ++v;
                seek();
                return *this;
            }
            iterator operator++(int) {
                auto previous = *this;
                ++*this;
                return previous;
            }
            bool operator==(const iterator& other) const {
                if (v == -1 || other.v == -1) return v == other.v;
                return owner == other.owner && c == other.c && v == other.v;
            }
            bool operator!=(const iterator& other) const { return !(*this == other); }
        };

        iterator begin() const { return iterator(owner, c); }
        iterator end() const { return iterator(); }
    };

   public:
    /**
     * @brief 盤面を所有し、壁以外のマスからグラフを構築する O(HW)
     * @pre 各行の長さが等しいこと。空の盤面や幅 0 の盤面もよい。
     */
    explicit GridGraph(std::vector<std::string> board, char wall = '#')
        : data(std::move(board)), h(static_cast<long long>(data.size())), w(data.empty() ? 0 : static_cast<long long>(data.front().size())) {
        for (const auto& row : data) assert(static_cast<long long>(row.size()) == w);
        const Grid grid(h, w);
        ids.assign(grid.size(), -1);
        for (auto [i, j] : grid) {
            if (data[i][j] == wall) continue;
            ids[grid.index(i, j)] = size();
            cells.emplace_back(i, j);
        }
        g = Graph<long long>(size());
        for (long long u = 0; u < size(); ++u) {
            const auto [i, j] = cell(u);
            for (auto [ni, nj] : grid.neighbors4(i, j)) {
                const long long v = index(ni, nj);
                // 壁 (-1) を除き、各無向辺を 1 回だけ追加する。
                if (u < v) g.add_edge(u, v);
            }
        }
    }

    const Graph<long long>& graph() const { return g; }
    long long size() const { return static_cast<long long>(cells.size()); }
    long long height() const { return h; }
    long long width() const { return w; }
    const std::vector<Graph<long long>::Edge>& operator[](long long v) const { return g[v]; }
    const std::vector<std::string>& board() const { return data; }

    /** @brief 座標に対応する頂点番号。壁なら -1、範囲外なら assert */
    long long index(long long i, long long j) const { return ids[Grid(h, w).index(i, j)]; }
    /** @brief 頂点番号に対応する座標 */
    Grid::value_type cell(long long v) const {
        assert(0 <= v && v < size());
        return cells[v];
    }
    /** @brief 頂点に対応する文字 */
    char label(long long v) const {
        const auto [i, j] = cell(v);
        return data[i][j];
    }
    /** @brief 文字 c に対応する最初の頂点番号。なければ -1。O(V) */
    long long find(char c) const {
        const auto matches = find_all(c);
        const auto it = matches.begin();
        return it == matches.end() ? -1 : *it;
    }
    /**
     * @brief 文字 c に対応する頂点番号を行優先で遅延列挙する。構築 O(1)、全走査 O(V)
     * @pre 走査中は元の GridGraph を有効なまま保持し、変更しないこと。
     */
    Matches find_all(char c) const { return Matches(this, c); }
};
