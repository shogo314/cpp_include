#pragma once
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

struct Range {
   public:
    using value_type = long long;
    using const_reference = const value_type&;
    using size_type = std::size_t;
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
     * @brief step != 0
     */
    constexpr Range(value_type _start, value_type _stop, value_type _step)
        : start(_start), stop(_stop), step(_step), norm_stop(normalize(_start, _stop, _step)) {
    }
    constexpr Range(value_type _start, value_type _stop)
        : start(_start), stop(_stop), step(1), norm_stop(normalize(_start, _stop, 1)) {}
    constexpr Range(value_type _stop)
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
            return range.start == o.range.start and range.stop == o.range.stop and range.step == o.range.step;
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
            return !a.same_range(b) or a.value != b.value;
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
    constexpr difference_type count(value_type x) const noexcept {
        if (step > 0) {
            if (start <= x and x < stop) {
                if ((x - start) % step == 0) {
                    return 1;
                }
            }
        } else {
            if (stop < x and x <= start) {
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
        assert(i < size());
        return start + i * step;
    }
    constexpr value_type operator[](size_type i) const {
        return at(i);
    }
    constexpr bool empty() const noexcept {
        return start == norm_stop;
    }
    constexpr value_type sum() const noexcept {
        return static_cast<Range::value_type>(size()) * (norm_stop - step + start) / 2;
    }
    /**
     * @brief 最小値
     * 空だとエラー
     */
    constexpr value_type min() const {
        assert(not empty());
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
        assert(not empty());
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
     * @brief 要素が [l,r] である範囲を返す
     */
    constexpr Range clipped(value_type l, value_type r) const {
        if (l > r) return {};
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
        if (r.min() >= static_cast<Range::value_type>(size())) return {};
        Range trimmed_range(r.clipped(0, static_cast<Range::value_type>(size()) - 1));
        if (trimmed_range.empty()) return {};
        value_type _step = step * trimmed_range.step;
        value_type _start = at(trimmed_range.start);
        value_type _stop = _start + _step * static_cast<Range::value_type>(trimmed_range.size());
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
        if (a.empty() and b.empty()) return true;
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
