#pragma once
// https://github.com/shogo314/shogo-library/blob/93de89301cdeac655ce53761092d9556752fd185/src/utility/range.hpp

/**
 * @file range.hpp
 * @brief Python の range のような整数列と、その直積の Rect
 */

#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

struct Range {
   public:
    using value_type = long long;
    using const_reference = const value_type&;
    using size_type = long long;
    using difference_type = std::ptrdiff_t;

    const value_type start;
    const value_type stop;
    const value_type step;

   private:
    const value_type norm_stop;

    constexpr static value_type normalize(value_type _start, value_type _stop, value_type _step) {
        assert(_step != 0);
        if (_step > 0) {
            if (_start >= _stop) {
                return _start;
            } else {
                return _start + (_stop - _start + _step - 1) / _step * _step;
            }
        } else {
            if (_start <= _stop) {
                return _start;
            } else {
                return _start - (_start - _stop - _step - 1) / _step * _step;
            }
        }
    }

    constexpr Range(value_type _start, value_type _stop, value_type _step, value_type _norm_stop)
        : start(_start), stop(_stop), step(_step), norm_stop(_norm_stop) {
    }

   public:
    /**
     * @brief start から stop の手前まで step ずつ進む整数列
     * @pre step != 0
     */
    constexpr Range(value_type _start, value_type _stop, value_type _step)
        : start(_start), stop(_stop), step(_step), norm_stop(normalize(_start, _stop, _step)) {
    }
    constexpr Range(value_type _start, value_type _stop)
        : start(_start), stop(_stop), step(1), norm_stop(normalize(_start, _stop, 1)) {}
    explicit constexpr Range(value_type _stop)
        : start(0), stop(_stop), step(1), norm_stop(normalize(0, _stop, 1)) {}
    constexpr Range()
        : start(0), stop(0), step(1), norm_stop(0) {
    }
    constexpr Range(const Range& o) = default;
    Range& operator=(const Range& o) = delete;

    struct const_iterator {
       public:
        using difference_type = Range::difference_type;
        using value_type = Range::value_type;
        using pointer = const Range::value_type*;
        using reference = const Range::value_type&;
        using iterator_category = std::random_access_iterator_tag;

       private:
        const Range& range;
        value_type value;

        constexpr bool same_range(const const_iterator& o) const noexcept {
            return range.start == o.range.start && range.stop == o.range.stop && range.step == o.range.step;
        }

       public:
        constexpr const_iterator(const Range& _range, value_type _value) noexcept
            : range(_range), value(_value) {}
        constexpr const_iterator(const const_iterator& o) noexcept
            : range(o.range), value(o.value) {}
        constexpr reference base() const noexcept {
            return value;
        }
        constexpr const_iterator& operator=(const const_iterator& o) {
            assert(same_range(o));
            value = o.value;
            return *this;
        };
        constexpr const_iterator& operator++() noexcept {
            value += range.step;
            return *this;
        }
        constexpr const_iterator operator++(int) noexcept {
            const_iterator ret(*this);
            value += range.step;
            return ret;
        }
        constexpr const_iterator& operator+=(difference_type _n) noexcept {
            value += _n * range.step;
            return *this;
        }
        constexpr friend const_iterator operator+(const const_iterator& i, difference_type _n) noexcept {
            const_iterator ret(i);
            ret += _n;
            return ret;
        }
        constexpr friend const_iterator operator+(difference_type _n, const const_iterator& i) noexcept {
            const_iterator ret(i);
            ret += _n;
            return ret;
        }
        constexpr const_iterator& operator--() noexcept {
            value -= range.step;
            return *this;
        }
        constexpr const_iterator operator--(int) noexcept {
            const_iterator ret(*this);
            value -= range.step;
            return ret;
        }
        constexpr const_iterator& operator-=(difference_type _n) noexcept {
            value -= _n * range.step;
            return *this;
        }
        constexpr friend const_iterator operator-(const const_iterator& i, difference_type _n) noexcept {
            const_iterator ret(i);
            ret -= _n;
            return ret;
        }
        constexpr reference operator*() const noexcept {
            return value;
        }
        constexpr friend bool operator!=(const const_iterator& a, const const_iterator& b) noexcept {
            return !a.same_range(b) || a.value != b.value;
        }
        constexpr friend bool operator==(const const_iterator& a, const const_iterator& b) noexcept {
            return !(a != b);
        }
        constexpr friend difference_type operator-(const const_iterator& a, const const_iterator& b) {
            assert(a.same_range(b));
            if (a.range.step > 0) {
                return (a.value - b.value) / a.range.step;
            } else {
                return (b.value - a.value) / (-a.range.step);
            }
        }
    };

    constexpr const_iterator begin() const noexcept {
        return {*this, start};
    }
    constexpr const_iterator end() const noexcept {
        return {*this, norm_stop};
    }
    /**
     * @brief x の個数 (0 か 1)
     */
    constexpr size_type count(value_type x) const noexcept {
        // step がコンパイル時に分からないときも、よく使う step = 1 では割り算をしない
        if (step == 1) return start <= x && x < stop;
        if (step > 0) {
            if (start <= x && x < stop) {
                if ((x - start) % step == 0) {
                    return 1;
                }
            }
        } else {
            if (stop < x && x <= start) {
                if ((start - x) % (-step) == 0) {
                    return 1;
                }
            }
        }
        return 0;
    }
    /**
     * @brief x が含まれるか
     */
    constexpr bool contains(value_type x) const noexcept {
        return count(x) != 0;
    }
    /**
     * @brief x のインデックス
     * x が含まれないとエラー
     */
    constexpr size_type index(value_type x) const {
        assert(contains(x));
        if (step == 1) return x - start;
        return (x - start) / step;
    }
    /**
     * @brief 要素が x のイテレータ
     * x が含まれないと end() を返す
     */
    constexpr const_iterator find(value_type x) const noexcept {
        if (contains(x)) {
            return {*this, x};
        } else {
            return {*this, norm_stop};
        }
    }
    /**
     * @brief 要素が x 以上の最小のイテレータ
     * step > 0 じゃないとエラー
     */
    constexpr const_iterator lower_bound(value_type x) const {
        assert(step > 0);
        if (norm_stop <= x) {
            return end();
        } else if (x < start) {
            return begin();
        } else {
            return {*this, start + (x - start + step - 1) / step * step};
        }
    }
    /**
     * @brief 要素が x より大きい最小のイテレータ
     * step > 0 じゃないとエラー
     */
    constexpr const_iterator upper_bound(value_type x) const {
        assert(step > 0);
        if (norm_stop <= x) {
            return end();
        } else if (x < start) {
            return begin();
        } else {
            return {*this, start + (x - start + step) / step * step};
        }
    }
    /**
     * @brief コンテナの要素数
     */
    constexpr size_type size() const noexcept {
        if (step > 0) {
            return (norm_stop - start) / step;
        } else {
            return (start - norm_stop) / (-step);
        }
    }
    constexpr value_type at(size_type i) const {
        assert(0 <= i && i < size());
        return start + i * step;
    }
    constexpr value_type operator[](size_type i) const {
        return at(i);
    }
    constexpr bool empty() const noexcept {
        return start == norm_stop;
    }
    constexpr value_type sum() const noexcept {
        return size() * (norm_stop - step + start) / 2;
    }
    /**
     * @brief 最小値
     * 空だとエラー
     */
    constexpr value_type min() const {
        assert(!empty());
        if (step > 0) {
            return start;
        } else {
            return norm_stop - step;
        }
    }
    /**
     * @brief 最大値
     * 空だとエラー
     */
    constexpr value_type max() const {
        assert(!empty());
        if (step > 0) {
            return norm_stop - step;
        } else {
            return start;
        }
    }
    /**
     * @brief 総 xor
     * 負を含むとエラー
     */
    constexpr value_type product_xor() const noexcept {
        if (empty()) return 0;
        assert(min() >= 0);
        Range sorted_range(sorted());
        value_type res = 0;
        std::make_unsigned_t<value_type> t = 1;
        while (t <= static_cast<std::make_unsigned_t<value_type>>(max())) {
            std::make_unsigned_t<value_type> tmp = 0, n = size(), m = t, a = sorted_range.step, b = sorted_range.start;
            while (true) {
                if (a >= m) {
                    tmp += n * (n - 1) / 2 * (a / m);
                    a %= m;
                }
                if (b >= m) {
                    tmp += n * (b / m);
                    b %= m;
                }
                std::make_unsigned_t<value_type> y_max = a * n + b;
                if (y_max < m) break;
                n = y_max / m;
                b = y_max % m;
                std::swap(m, a);
            }
            if (tmp % 2 == 1) {
                res += t;
            }
            t <<= 1;
        }
        return res;
    }
    /**
     * @brief 逆順にしたもの
     */
    constexpr Range reversed() const noexcept {
        return {norm_stop - step, start - step, -step, start - step};
    }
    /**
     * @brief step > 0 だとそのまま step < 0 だと逆順にしたもの
     */
    constexpr Range sorted() const noexcept {
        if (step > 0) {
            return {start, norm_stop, step, norm_stop};
        } else {
            return {norm_stop - step, start - step, -step, start - step};
        }
    }
    /**
     * @brief 要素が [l, r) である範囲を返す
     */
    constexpr Range clipped(value_type l, value_type r) const {
        if (l >= r) return {};
        r--;  // 以下では r を含む [l, r] として扱う
        if (step > 0) {
            value_type _start = [&] {
                if (norm_stop <= l) {
                    return norm_stop;
                } else if (l < start) {
                    return start;
                } else {
                    return start + (l - start + step - 1) / step * step;
                }
            }();
            value_type _stop = [&] {
                if (norm_stop <= r) {
                    return norm_stop;
                } else if (r < start) {
                    return start;
                } else {
                    return start + (r - start + step) / step * step;
                }
            }();
            return {_start, _stop, step, _stop};
        } else {
            value_type _start = [&] {
                if (start - step <= r) {
                    return start;
                } else if (r < norm_stop - step) {
                    return norm_stop;
                } else {
                    return norm_stop + (r - norm_stop) / step * step;
                }
            }();
            value_type _stop = [&] {
                if (start - step <= l) {
                    return start;
                } else if (l < norm_stop - step) {
                    return norm_stop;
                } else {
                    return norm_stop + (l - norm_stop - 1) / step * step;
                }
            }();
            return {_start, _stop, step, _stop};
        }
    }
    /**
     * @brief 要素を Range で指定したコンテナ
     */
    constexpr Range slice(const Range& r) const {
        if (r.empty()) return {};
        if (r.max() < 0) return {};
        if (r.min() >= size()) return {};
        Range trimmed_range(r.clipped(0, size()));
        if (trimmed_range.empty()) return {};
        value_type _step = step * trimmed_range.step;
        value_type _start = at(trimmed_range.start);
        value_type _stop = _start + _step * trimmed_range.size();
        return {_start, _stop, _step, _stop};
    }

    static std::string type_str() {
        return "Range";
    }
    std::string initializer_str() const {
        return "{" + std::to_string(start) + ", " + std::to_string(stop) + ", " + std::to_string(step) + "}";
    }
    std::string repr() const {
        return "Range(" + std::to_string(start) + ", " + std::to_string(stop) + ", " + std::to_string(step) + ")";
    }

    std::vector<value_type> to_vector() const {
        std::vector<value_type> ret;
        ret.reserve(size());
        if (step > 0) {
            for (value_type x = start; x < stop; x += step) {
                ret.push_back(x);
            }
        } else {
            for (value_type x = start; x > stop; x += step) {
                ret.push_back(x);
            }
        }
        return ret;
    }
    constexpr friend bool operator==(const Range& a, const Range& b) noexcept {
        if (a.empty() && b.empty()) return true;
        if (a.size() != b.size()) return false;
        if (a.start != b.start) return false;
        if (a.size() == 1) return true;
        if (a.step != b.step) return false;
        return true;
    }
    constexpr friend bool operator!=(const Range& a, const Range& b) noexcept {
        return !(a == b);
    }
};

/**
 * @brief 2 つの Range の直積 rows × cols
 * (i, j) を i の昇順、同じ i では j の昇順 (行優先) に並べたものとして扱う
 */
struct Rect {
   public:
    using value_type = std::pair<long long, long long>;
    using size_type = long long;
    using difference_type = std::ptrdiff_t;

    const Range rows;
    const Range cols;

   private:
    const long long width;  // cols.size() (index や at で毎回割り算しないように持っておく)

   public:
    /**
     * @brief [0, h) × [0, w)
     */
    constexpr Rect(long long h, long long w) : rows(h), cols(w), width(cols.size()) {}
    constexpr Rect(const Range& _rows, const Range& _cols) : rows(_rows), cols(_cols), width(cols.size()) {}
    constexpr Rect(const Rect& o) = default;
    Rect& operator=(const Rect& o) = delete;

    struct const_iterator {
       public:
        using difference_type = Rect::difference_type;
        using value_type = Rect::value_type;
        using pointer = void;
        using reference = value_type;
        using iterator_category = std::forward_iterator_tag;

       private:
        const Rect* rect = nullptr;
        long long w = 0;   // cols.size()
        long long ic = 0;  // rows の何番目か
        long long jc = 0;  // cols の何番目か

       public:
        constexpr const_iterator() noexcept = default;
        constexpr const_iterator(const Rect* _rect, long long _w, long long _ic, long long _jc) noexcept
            : rect(_rect), w(_w), ic(_ic), jc(_jc) {}
        constexpr value_type operator*() const noexcept {
            return {rect->rows.start + ic * rect->rows.step, rect->cols.start + jc * rect->cols.step};
        }
        constexpr const_iterator& operator++() noexcept {
            if (++jc == w) {
                jc = 0;
                ++ic;
            }
            return *this;
        }
        constexpr const_iterator operator++(int) noexcept {
            const_iterator ret(*this);
            ++*this;
            return ret;
        }
        constexpr friend bool operator==(const const_iterator& a, const const_iterator& b) noexcept {
            return a.ic == b.ic && a.jc == b.jc;
        }
        constexpr friend bool operator!=(const const_iterator& a, const const_iterator& b) noexcept {
            return !(a == b);
        }
    };

    constexpr const_iterator begin() const noexcept {
        return {this, width, width == 0 ? rows.size() : 0, 0};
    }
    constexpr const_iterator end() const noexcept {
        return {this, width, rows.size(), 0};
    }
    /**
     * @brief 要素数 rows.size() * cols.size()
     */
    constexpr size_type size() const noexcept {
        return rows.size() * width;
    }
    constexpr bool empty() const noexcept {
        return rows.empty() || cols.empty();
    }
    /**
     * @brief (i, j) が含まれるか
     */
    constexpr bool contains(long long i, long long j) const noexcept {
        return rows.contains(i) && cols.contains(j);
    }
    constexpr bool contains(const value_type& p) const noexcept {
        return contains(p.first, p.second);
    }
    /**
     * @brief (i, j) の行優先での位置 (グリッドの頂点番号に使う)
     * (i, j) が含まれないとエラー
     */
    constexpr size_type index(long long i, long long j) const {
        assert(contains(i, j));
        return rows.index(i) * width + cols.index(j);
    }
    constexpr size_type index(const value_type& p) const {
        return index(p.first, p.second);
    }
    /**
     * @brief 行優先で k 番目の要素 (index の逆)
     */
    constexpr value_type at(size_type k) const {
        assert(0 <= k && k < size());
        return {rows[k / width], cols[k % width]};
    }
    constexpr value_type operator[](size_type k) const {
        return at(k);
    }

   private:
    // 近傍の方向 (rows・cols の中での位置の差) を Rect の順に並べたもの。先頭 4 つは上下左右だけを並べた別の表
    // 4 近傍と 8 近傍の境目が分かるように 2 行に分けているので整形しない
    // clang-format off
    static constexpr int neighbor_dirs[12][2] = {{-1, 0}, {0, -1}, {0, 1}, {1, 0},
                                                 {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    // clang-format on

   public:
    /**
     * @brief neighbors4 / neighbors8 の戻り値 (範囲 for で使う)
     * 作るときに Rect に含まれる方向をビットで持っておき、立っているビットを順に返す
     */
    struct Neighbors {
        long long i, j, row_step, col_step;
        int first;      // neighbor_dirs[first + k] がビット k の方向
        unsigned mask;  // Rect に含まれる方向

        struct iterator {
            using difference_type = std::ptrdiff_t;
            using value_type = Rect::value_type;
            using pointer = void;
            using reference = value_type;
            using iterator_category = std::forward_iterator_tag;

            const Neighbors* nb = nullptr;
            unsigned rest = 0;  // まだ返していない方向

            constexpr value_type operator*() const noexcept {
                const int k = nb->first + __builtin_ctz(rest);
                return {nb->i + neighbor_dirs[k][0] * nb->row_step, nb->j + neighbor_dirs[k][1] * nb->col_step};
            }
            constexpr iterator& operator++() noexcept {
                rest &= rest - 1;
                return *this;
            }
            constexpr iterator operator++(int) noexcept {
                iterator ret(*this);
                ++*this;
                return ret;
            }
            constexpr friend bool operator==(const iterator& a, const iterator& b) noexcept { return a.rest == b.rest; }
            constexpr friend bool operator!=(const iterator& a, const iterator& b) noexcept { return a.rest != b.rest; }
        };
        constexpr iterator begin() const noexcept { return {this, mask}; }
        constexpr iterator end() const noexcept { return {this, 0}; }
        constexpr long long size() const noexcept { return __builtin_popcount(mask); }
        constexpr bool empty() const noexcept { return mask == 0; }
    };

   private:
    constexpr Neighbors neighbors(long long i, long long j, int first, int last) const {
        assert(contains(i, j));
        unsigned mask = 0;
        for (int k = first; k < last; k++) {
            if (contains(i + neighbor_dirs[k][0] * rows.step, j + neighbor_dirs[k][1] * cols.step)) mask |= 1u << (k - first);
        }
        return {i, j, rows.step, cols.step, first, mask};
    }

   public:
    /**
     * @brief (i, j) の上下左右のうち Rect に含まれるものを、Rect の順に返す
     * 隣は rows・cols の中で隣り合う要素 (i ± rows.step など)。(i, j) が含まれないとエラー
     */
    constexpr Neighbors neighbors4(long long i, long long j) const {
        return neighbors(i, j, 0, 4);
    }
    constexpr Neighbors neighbors4(const value_type& p) const {
        return neighbors4(p.first, p.second);
    }
    /**
     * @brief (i, j) を囲む 8 つのうち Rect に含まれるものを、Rect の順に返す
     */
    constexpr Neighbors neighbors8(long long i, long long j) const {
        return neighbors(i, j, 4, 12);
    }
    constexpr Neighbors neighbors8(const value_type& p) const {
        return neighbors8(p.first, p.second);
    }

    static std::string type_str() {
        return "Rect";
    }
    std::string initializer_str() const {
        return "{" + rows.initializer_str() + ", " + cols.initializer_str() + "}";
    }
    std::string repr() const {
        return "Rect(" + rows.repr() + ", " + cols.repr() + ")";
    }

    constexpr friend bool operator==(const Rect& a, const Rect& b) noexcept {
        if (a.empty() && b.empty()) return true;
        return a.rows == b.rows && a.cols == b.cols;
    }
    constexpr friend bool operator!=(const Rect& a, const Rect& b) noexcept {
        return !(a == b);
    }
};
