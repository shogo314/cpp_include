#pragma once
// https://github.com/shogo314/shogo-library/blob/a60d38e806cfff5b358d4b2185661826b189fb6e/src/math/xor-basis.hpp

/**
 * @file xor-basis.hpp
 * @brief XOR 基底 (F2 上の線形基底)
 */

#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>

/**
 * @brief XOR 基底 (F2 上の線形基底)
 *
 * 追加した値の XOR で作れる値の集合 (線形空間) を管理する。各操作 O(B) (B は T のビット数)
 * 基底は、最上位ビットがすべて異なり、各基底の最上位ビットがほかの基底で 0 になる形 (簡約された形) に保つ
 * 例:
 * XorBasis<> xb;
 * xb.add(5); xb.add(3);
 * xb.contains(6);  // true
 * xb.max();        // 6
 *
 * @tparam T 64 ビット以下の符号なし整数型
 */
template <typename T = unsigned long long>
class XorBasis {
    static_assert(std::is_unsigned_v<T>, "XorBasis: T must be unsigned");
    static constexpr int B = std::numeric_limits<T>::digits;
    static_assert(B <= 64, "XorBasis: T must be at most 64 bits");
    T b[B] = {};  // b[i] は最上位ビットが i の基底 (なければ 0)
    int r = 0;

   public:
    XorBasis() = default;

    /**
     * @brief x を追加する。x が今の空間に含まれていなければ (基底が増えれば) true
     */
    bool add(T x) {
        for (int i = B - 1; i >= 0; i--) {
            if ((x >> i & 1) && b[i]) x ^= b[i];
        }
        if (x == 0) return false;
        int t = B - 1;
        while (!(x >> t & 1)) t--;
        // ほかの基底から、ビット t を消す
        for (int i = t + 1; i < B; i++) {
            if (b[i] >> t & 1) b[i] ^= x;
        }
        b[t] = x;
        r++;
        return true;
    }

    /**
     * @brief 基底の個数 (空間の次元)。作れる値は 2^rank() 個
     */
    long long rank() const { return r; }

    /**
     * @brief x を作れるか
     */
    bool contains(T x) const { return min(x) == 0; }

    /**
     * @brief 作れる値 y について x ^ y の最大値 (x = 0 なら作れる値の最大値)
     */
    T max(T x = 0) const {
        for (int i = B - 1; i >= 0; i--) {
            if (b[i] && !(x >> i & 1)) x ^= b[i];
        }
        return x;
    }

    /**
     * @brief 作れる値 y について x ^ y の最小値 (x = 0 なら 0)
     */
    T min(T x) const {
        for (int i = B - 1; i >= 0; i--) {
            if (x >> i & 1) x ^= b[i];
        }
        return x;
    }

    /**
     * @brief 作れる値のうち、小さい方から k 番目 (0-indexed) の値。k < 2^rank() であること
     */
    T kth(unsigned long long k) const {
        assert(r >= 64 || k < (1ULL << r));
        T res = 0;
        for (int i = 0; i < B && k; i++) {
            if (!b[i]) continue;
            if (k & 1) res ^= b[i];
            k >>= 1;
        }
        return res;
    }

    /**
     * @brief 作れる値のうち、x より小さいものの個数 (kth の逆)
     */
    unsigned long long count_less(T x) const {
        // 作れる値 y を上のビットから x と比べる。j はまだ選んでいない基底 (最上位ビットが i 未満) の個数
        unsigned long long res = 0;
        int j = r;
        T y = 0;
        for (int i = B - 1; i >= 0; i--) {
            if (b[i]) {
                // 簡約された形なので、y のビット i は b[i] を使うかどうかだけで決まる
                j--;
                if ((y >> i & 1) != (x >> i & 1)) y ^= b[i];
                // ビット i を 0 にした側の 2^j 個は、すべて x より小さい
                if (x >> i & 1) res += 1ULL << j;
            } else if ((y >> i & 1) != (x >> i & 1)) {
                // ビット i は上のビットの選び方で決まっていて、ここで大小が決まる
                if (x >> i & 1) res += 1ULL << j;
                return res;
            }
        }
        return res;
    }

    /**
     * @brief 基底 (最上位ビットの降順)
     */
    std::vector<T> basis() const {
        std::vector<T> res;
        for (int i = B - 1; i >= 0; i--) {
            if (b[i]) res.push_back(b[i]);
        }
        return res;
    }
};
