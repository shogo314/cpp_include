#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

struct Range {
   public:
    using value_type = long long;
    struct iterator {
        const value_type step;
        value_type value;
        constexpr iterator(value_type __step, value_type __value) noexcept
            : step(__step), value(__value) {
        }
        constexpr value_type operator*() const noexcept {
            return value;
        }
        constexpr iterator& operator++() noexcept {
            value += step;
            return *this;
        }
        constexpr friend bool operator!=(const iterator& a, const iterator& b) noexcept {
            return a.value != b.value;
        }
    };

    const value_type start;
    const value_type step;
    const value_type stop;
    constexpr Range(value_type __start, value_type __stop, value_type __step)
        : start(__start), stop(__stop), step(__step) {
        assert(__step != 0);
    }
    constexpr Range(value_type __start, value_type __stop) noexcept : start(__start), stop(__stop), step(1) {}
    constexpr Range(value_type __stop) noexcept : start(0), stop(__stop), step(1) {}
    constexpr iterator begin() const noexcept {
        Range&& norm = normalize();
        return {norm.step, norm.start};
    }
    constexpr iterator end() const noexcept {
        Range&& norm = normalize();
        return {norm.step, norm.stop};
    }
    constexpr size_t count(value_type x) const noexcept {
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
    constexpr bool contains(value_type x) const noexcept {
        return count(x) != 0;
    }
    constexpr size_t index(value_type x) const {
        assert(contains(x));
        if (step > 0) {
            return (x - start) / step;
        } else {
            return (start - x) / (-step);
        }
    }
    constexpr size_t size() const noexcept {
        Range&& norm = normalize();
        if (norm.step > 0) {
            return (norm.stop - norm.start) / norm.step;
        } else {
            return (norm.start - norm.stop) / (-norm.step);
        }
    }
    constexpr value_type at(size_t i) const {
        assert(0 <= i and i < size());
        return start + i * step;
    }
    constexpr value_type operator[](size_t i) const {
        return at(i);
    }
    constexpr value_type __sum__() const noexcept {
        Range&& norm = normalize();
        value_type ret = 0;
        value_type l = size();
        return l * (l - 1) / 2 * norm.step + l * norm.start;
    }
    constexpr Range __reversed__() const noexcept {
        Range&& norm = normalize();
        if (norm.start == norm.stop) {
            return {norm.start, norm.start, -norm.step};
        }
        return {norm.stop - norm.step, norm.start - norm.step, -norm.step};
    }
    std::string __repr__() const {
        using std::string, std::to_string;
        return string("Range(") + to_string(start) + ", " + to_string(stop) + ", " + to_string(step) + string(")");
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

    constexpr Range normalize() const noexcept {
        if (step > 0) {
            if (start >= stop) {
                return {start, start, step};
            } else {
                return {start, start + (stop - start + step - 1) / step * step, step};
            }
        } else {
            if (start <= stop) {
                return {start, start, step};
            } else {
                return {start, start - (start - stop - step - 1) / (-step) * (-step), step};
            }
        }
    }
};
