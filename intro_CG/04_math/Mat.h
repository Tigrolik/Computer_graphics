#ifndef MAT_H
#define MAT_H

#include <array>
#include <fstream>
#include "Algebra_lib.h"

namespace Algebra_lib {

template <int R, int C, class Num>
class Mat {
    static_assert(R > 0 && C > 0, "R and C must be positive");
    static_assert(R <= max_vec_size && C <= max_vec_size,
            "R and C must not exceed max_vec_size");
public:
    // ctors
    explicit Mat(const Num = Num{});
    explicit Mat(const Vec<C, Num>&); // fill each matrix row with a Vec
    explicit Mat(const std::array<std::array<Num, C>, R>&);
    explicit Mat(const std::initializer_list<Vec<C, Num>>&);

    // copy ctors and assignment operators
    explicit Mat(const Mat&) = default;
    template <class Comp>
        explicit Mat(const Mat<R, C, Comp>&);
    Mat& operator=(const Mat&) = default;
    template <class Comp>
        Mat& operator=(const Mat<R, C, Comp>&);

    // move ctor and assignment operator
    explicit Mat(Mat&&) = default;
    Mat& operator=(Mat&&) = default;

    // dtor
    ~Mat() = default;

    using value_type = Num;
    using iterator = typename Vec<R, Vec<C, Num>>::iterator;
    using const_iterator = typename Vec<R, Vec<C, Num>>::const_iterator;
    // iterators: using in loops (for (auto m: M))
    iterator begin() { return M_.begin(); }
    const_iterator begin() const { return M_.begin(); }
    const_iterator cbegin() const { return M_.cbegin(); }
    iterator end() { return M_.end(); }
    const_iterator end() const { return M_.end(); }
    const_iterator cend() const { return M_.cend(); }


private:
    Vec<R, Vec<C, Num>> M_;
};

template <int R, int C, class Num>
Mat<R, C, Num>::Mat(const Num val): M_{} {
    for (auto &m: M_)
        m = Vec<C, Num>(val);
}

template <int R, int C, class Num>
Mat<R, C, Num>::Mat(const Vec<C, Num>& v): M_{} {
    for (auto &m: M_)
        m = v;
}

template <int R, int C, class Num>
Mat<R, C, Num>::Mat(const std::array<std::array<Num, C>, R> &M): M_{} {
    for (auto i = 0; i < R; ++i)
        M_[i] = M[i];
}

template <int R, int C, class Num>
Mat<R, C, Num>::Mat(const std::initializer_list<Vec<C, Num>> &IL): M_{} {
    auto iter = std::begin(IL);
    for (auto i = 0; i < std::min<int>(R, IL.size()); ++i, ++iter)
        M_[i] = *iter;
}


// output operator
template <int R, int C, class Num>
std::ostream& operator<<(std::ostream &os, const Mat<R, C, Num> &M) {
    os << "{\n";
    for (const auto &m: M) os << m << '\n';
    return os << '}';
}

} // Algebra_lib

#endif /* MAT_H */

