#pragma once
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/data_structures/fast-set.hpp

/**
 * @file fast-set.hpp
 * @brief 64 分木による整数の集合 (FastSet)
 */

#include <cassert>
#include <cstdint>
#include <vector>

/**
 * @brief [0, n) の整数の集合を 64 分木で持つ
 *
 * 各段は 64 bit の配列で、上の段の bit は下の段の 64 bit が 0 でないかを表す
 * 挿入・削除・x 以上の最小・x 以下の最大を O(log_64 n) で行う (n = 10^9 でも 5 段程度)
 * 例:
 * FastSet s(n);
 * s.insert(3); s.insert(10);
 * s.next(4);  // 10
 * s.prev(4);  // 3
 */
class FastSet {
    long long n, cnt = 0;
    std::vector<std::vector<std::uint64_t>> seg;  // seg[0] が要素の bit、seg[h + 1] は seg[h] の各 64 bit が 0 でないか

    void build_levels() {
        // 1 つの 64 bit に収まるまで段を重ねる
        do {
            long long m = seg.empty() ? n : static_cast<long long>(seg.back().size());
            seg.emplace_back((m + 63) / 64, 0);
        } while (seg.back().size() > 1);
    }

   public:
    /**
     * @brief [0, n) の空の集合
     */
    explicit FastSet(long long n = 0) : n(n) {
        assert(n >= 0);
        build_levels();
    }
    /**
     * @brief v[i] が true の i からなる集合 O(n)
     */
    explicit FastSet(const std::vector<bool>& v) : FastSet(static_cast<long long>(v.size())) {
        for (long long i = 0; i < n; i++) {
            if (v[i]) seg[0][i >> 6] |= std::uint64_t(1) << (i & 63), cnt++;
        }
        for (int h = 0; h + 1 < static_cast<int>(seg.size()); h++) {
            for (long long i = 0; i < static_cast<long long>(seg[h].size()); i++) {
                if (seg[h][i]) seg[h + 1][i >> 6] |= std::uint64_t(1) << (i & 63);
            }
        }
    }

    /**
     * @brief 扱える整数の範囲 [0, n) の n
     */
    long long limit() const { return n; }
    /**
     * @brief 要素数
     */
    long long size() const { return cnt; }
    /**
     * @brief 空か
     */
    bool empty() const { return cnt == 0; }

    /**
     * @brief x が含まれるか O(1)
     */
    bool contains(long long x) const {
        assert(0 <= x && x < n);
        return seg[0][x >> 6] >> (x & 63) & 1;
    }
    /**
     * @brief x の個数 (0 か 1) O(1)
     */
    long long count(long long x) const { return contains(x); }
    /**
     * @brief x を挿入する。新しく入ったら true O(log_64 n)
     */
    bool insert(long long x) {
        assert(0 <= x && x < n);
        if (contains(x)) return false;
        cnt++;
        for (auto& level : seg) {
            level[x >> 6] |= std::uint64_t(1) << (x & 63);
            x >>= 6;
        }
        return true;
    }
    /**
     * @brief x を削除する。入っていたら true O(log_64 n)
     */
    bool erase(long long x) {
        assert(0 <= x && x < n);
        if (!contains(x)) return false;
        cnt--;
        for (auto& level : seg) {
            level[x >> 6] &= ~(std::uint64_t(1) << (x & 63));
            // 同じ 64 bit に他の要素が残っていれば、上の段は変わらない
            if (level[x >> 6]) break;
            x >>= 6;
        }
        return true;
    }

    /**
     * @brief x 以上の最小の要素 (なければ -1) O(log_64 n)
     */
    long long next(long long x) const {
        if (x < 0) x = 0;
        if (x >= n) return -1;
        for (int h = 0; h < static_cast<int>(seg.size()); h++) {
            if ((x >> 6) == static_cast<long long>(seg[h].size())) break;
            std::uint64_t d = seg[h][x >> 6] >> (x & 63);
            if (!d) {
                x = (x >> 6) + 1;
                continue;
            }
            // この段で見つかったので、下の段では最も小さい bit をたどる
            x += __builtin_ctzll(d);
            for (int g = h - 1; g >= 0; g--) {
                x <<= 6;
                x += __builtin_ctzll(seg[g][x >> 6]);
            }
            return x;
        }
        return -1;
    }
    /**
     * @brief x 以下の最大の要素 (なければ -1) O(log_64 n)
     */
    long long prev(long long x) const {
        if (x >= n) x = n - 1;
        if (x < 0) return -1;
        for (int h = 0; h < static_cast<int>(seg.size()); h++) {
            if (x == -1) break;
            std::uint64_t d = seg[h][x >> 6] << (63 - (x & 63));
            if (!d) {
                x = (x >> 6) - 1;
                continue;
            }
            // この段で見つかったので、下の段では最も大きい bit をたどる
            x -= __builtin_clzll(d);
            for (int g = h - 1; g >= 0; g--) {
                x <<= 6;
                x += 63 - __builtin_clzll(seg[g][x >> 6]);
            }
            return x;
        }
        return -1;
    }
    /**
     * @brief 最小の要素 (空なら -1) O(log_64 n)
     */
    long long min() const { return next(0); }
    /**
     * @brief 最大の要素 (空なら -1) O(log_64 n)
     */
    long long max() const { return prev(n - 1); }

    /**
     * @brief 要素を昇順に並べたもの O(size() log_64 n)
     */
    std::vector<long long> to_vector() const {
        std::vector<long long> res;
        res.reserve(cnt);
        for (long long x = next(0); x != -1; x = next(x + 1)) res.push_back(x);
        return res;
    }
    /**
     * @brief すべて削除する O(n / 64)
     */
    void clear() {
        for (auto& level : seg) level.assign(level.size(), 0);
        cnt = 0;
    }
};
