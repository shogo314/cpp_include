#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <tuple>

class Range {
   public:
    using value_type = long long;
    const value_type start;
    const value_type step;
    const value_type stop;
    Range(value_type __start, value_type __stop, value_type __step)
        : start(__start), stop(__stop), step(__step) {
        assert(__step != 0);
    }
    Range(value_type __start, value_type __stop) : start(__start), stop(__stop), step(1) {}
    Range(value_type __stop) : start(0), stop(__stop), step(1) {}
    size_t count() const {
        return 0;
    }
    size_t index(value_type x) const {
        return 0;
    }
    size_t size() const {
    }
    value_type at(size_t i) const {
        assert(0 <= i and i < size());
        return 0;
    }
    bool contains(value_type x) const {
    }
    size_t __len__() const {
    }
    value_type __sum__() const {
    }
    Range __reversed__() const {
    }
    std::string __repr__() const {
        using std::string, std::to_string;
        return string("Range(") + to_string(start) + ", " + to_string(stop) + ", " + to_string(step) + string(")");
    }

   private:
    std::tuple<value_type, value_type, value_type> normalize() const {
        if (step > 0) {
            if (start >= stop) {
                return {start, start, step};
            } else {
                return {start, start + (stop - start + step - 1) / step * step, step};
            }
        } else {
            if (start <= step) {
                return {start, start, step};
            }
        }
    }
};
