#ifndef MAT_H
#define MAT_H

#include "Algebra_lib.h"
#include <array>
#include <fstream>

namespace Algebra_lib {

template <size_t R, size_t C, class Num>
class Mat {
    static_assert(R > 0 && C > 0, "Mat sizes must be positive");
    static_assert(R <= max_vec_size && C <= max_vec_size,
            "Mat sizes must not exceed max_vec_size");
public:
    /*
     * Ctors, assignment operators and dtor
     */
    // ctors
    constexpr Mat();
    explicit Mat(const std::array<std::array<Num, C>, R>&);
    explicit Mat(const std::initializer_list<Vec<C, Num>>&);

    // copy ctors
    constexpr Mat(const Mat&) = default;
    //template <class Comp>
    //    Mat(const Mat<R, C, Comp>&);

    // assignment operators
    Mat& operator=(const Mat&) = default;
    //template <class Comp>
    //    Mat& operator=(const Mat<R, C, Comp>&);

    // move ctor and assignment
    Mat(Mat&&) = default;
    Mat& operator=(Mat&&) = default;

    // dtor
    ~Mat() = default;
    /* --- end ctors, assignment operators and dtor --- */

    // cast operators
    explicit operator Mat<R, C, double>() const;

    /*
     * Compound arithemtic operators
     */
    Mat& operator+=(const Num&);
    Mat& operator+=(const Mat&);
    Mat& operator-=(const Num&);
    Mat& operator-=(const Mat&);
    Mat& operator*=(const Num&);
    Mat& operator/=(const Num&);
    /* --- end compound arithemtic operators --- */

    using value_type     = Num;
    using size_type      = typename Vec<C, Num>::size_type;
    using iterator       = typename Vec<R, Vec<C, Num>>::iterator;
    using const_iterator = typename Vec<R, Vec<C, Num>>::const_iterator;

    // iterators: using in loops (for (auto m: M))
    iterator begin() { return M_.begin(); }
    const_iterator begin() const { return M_.begin(); }
    const_iterator cbegin() const { return M_.cbegin(); }
    iterator end() { return M_.end(); }
    const_iterator end() const { return M_.end(); }
    const_iterator cend() const { return M_.cend(); }

    // index and range-check index operators
    Vec<C, Num>& operator[](const size_type i) { return M_[i]; }
    const Vec<C, Num>& operator[](const size_type i) const { return M_[i]; }
    Vec<C, Num>& at(const size_type i) { return M_.at(i); }
    const Vec<C, Num>& at(const size_type i) const { return M_.at(i); }
    // row and column functions
    Vec<C, Num>& row(const size_type i) { return Mat::at(i); }
    const Vec<C, Num>& row(const size_type i) const { return Mat::at(i); }
    Vec<R, Num> col(const size_type);
    const Vec<R, Num> col(const size_type) const;

    // fill a matrix with a value or a row/column with a given vector
    void fill(const Num&);
    void fill_row(const size_type, const Vec<C, Num>&);
    void fill_col(const size_type, const Vec<R, Num>&);
    // fill each matrix row/vector with a given vector
    void fill_each_row(const Vec<C, Num>&);
    void fill_each_col(const Vec<R, Num>&);

    const Mat<C, R, Num> transpose() const;

    // size methods
    constexpr size_t nrows() { return R; }
    constexpr size_t ncols() { return C; }
    constexpr Vec<2, size_type> size() const {
        return Vec<2, size_type> {R, C};
    }

private:
    Vec<R, Vec<C, Num>> M_;
};

/*
 * Ctors and assignment operators
 */
// default constructor
template <size_t R, size_t C, class Num>
constexpr Mat<R, C, Num>::Mat(): M_{} { }

// constructor with std::array
template <size_t R, size_t C, class Num>
Mat<R, C, Num>::Mat(const std::array<std::array<Num, C>, R> &M): M_{} {
    for (size_t i {R}; i--; M_[i] = Vec<C, Num>{M[i]}) { }
}

// constructor with initializer_list
template <size_t R, size_t C, class Num>
Mat<R, C, Num>::Mat(const std::initializer_list<Vec<C, Num>> &IL): M_{} {
    auto iter = std::begin(IL);
    for (size_t i {0}; i < std::min(R, IL.size()); ++i, ++iter)
        M_[i] = *iter;
}
/* --- end ctors and assignment operators --- */

// cast operators
// convert a vector into a double vector
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num>::operator Mat<R, C, double>() const {
    Mat<R, C, double> res {};
    for (size_t i {0}; i < R; ++i)
        res[i] = Vec<C, double>(M_[i]);
    return res;
}

/*
 * Index methods
 */
// get column as a vector by index
template <size_t R, size_t C, class Num>
inline Vec<R, Num> Mat<R, C, Num>::col(const size_type j) {
    Vec<R, Num> v;
    for (auto i = R; i--; v[i] = M_[i][j]) { }
    return v;
}

template <size_t R, size_t C, class Num>
inline const Vec<R, Num> Mat<R, C, Num>::col(const size_type j) const {
    Vec<R, Num> v;
    for (auto i = R; i--; v[i] = M_[i][j]) { }
    return v;
}
/* --- end index methods --- */

/*
 * Compound arithemtic operators
 */
// add a value to a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num>& Mat<R, C, Num>::operator+=(const Num &rhs) {
    for (auto &m: M_)
        m += rhs;
    return *this;
}

// add a matrix to a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num>& Mat<R, C, Num>::operator+=(const Mat<R, C, Num> &rhs) {
    for (size_type i {0}; i < R; ++i)
        M_[i] += rhs[i];
    return *this;
}

// subtract a value from a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num>& Mat<R, C, Num>::operator-=(const Num &rhs) {
    for (auto &m: M_)
        m -= rhs;
    return *this;
}

// subtract a matrix from a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num>& Mat<R, C, Num>::operator-=(const Mat<R, C, Num> &rhs) {
    for (size_type i {0}; i < R; ++i)
        M_[i] -= rhs[i];
    return *this;
}

// multiply a matrix by a value
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num>& Mat<R, C, Num>::operator*=(const Num &rhs) {
    for (auto &m: M_)
        m *= rhs;
    return *this;
}

// divide a matrix by a value
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num>& Mat<R, C, Num>::operator/=(const Num &rhs) {
    for (auto &m: M_)
        m /= rhs;
    return *this;
}
/* --- end compound arithemtic operators --- */

/*
 * ------------------ Mat arithmetic operators ------------------
 */
// add a value to a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num> operator+(const Mat<R, C, Num> &lhs, const Num &rhs) {
    Mat<R, C, Num> M {lhs};
    return M += rhs;
}

// add a matrix to a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num> operator+(const Mat<R, C, Num> &lhs,
        const Mat<R, C, Num> &rhs) {
    Mat<R, C, Num> M {lhs};
    return M += rhs;
}

// subtract a value from a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num> operator-(const Mat<R, C, Num> &lhs, const Num &rhs) {
    Mat<R, C, Num> M {lhs};
    return M -= rhs;
}

// subtract a matrix from a matrix
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num> operator-(const Mat<R, C, Num> &lhs,
        const Mat<R, C, Num> &rhs) {
    Mat<R, C, Num> M {lhs};
    return M -= rhs;
}

// multiply a matrix by a value
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num> operator*(const Mat<R, C, Num> &lhs, const Num &rhs) {
    Mat<R, C, Num> M {lhs};
    return M *= rhs;
}

// divide a matrix by a value
template <size_t R, size_t C, class Num>
inline Mat<R, C, Num> operator/(const Mat<R, C, Num> &lhs, const Num &rhs) {
    Mat<R, C, Num> M {lhs};
    return M /= rhs;
}

// multiply matrix by a vector: output is a vector
template <size_t R, size_t C, class Num>
inline Vec<R, Num> operator*(const Mat<R, C, Num> &lhs, const Vec<C, Num> &v) {
    Vec<R, Num> res;
    for (size_t i {0}; i < R; ++i)
        res[i] = dot(lhs[i], v);
    return res;
}

// multiply a vector by a matrix: output is a vector
template <size_t R, size_t C, class Num>
inline Vec<C, Num> operator*(const Vec<R, Num> &lhs,const Mat<R, C, Num> &M) {
    Vec<C, Num> res;
    for (size_t i {0}; i < C; ++i)
        res[i] = dot(lhs, M.col(i));
    return res;
}

// multiply matrix by a matrix
template <size_t R, size_t RC, size_t C, class Num>
inline Mat<R, C, Num> operator*(const Mat<R, RC, Num> &lhs,
        const Mat<RC, C, Num> &rhs) {
    Mat<R, C, Num> M;
    for (size_t i {0}; i < R; ++i)
        for (size_t j {0}; j < C; ++j)
            M[i][j] = dot(lhs[i], rhs.col(j));
    return M;
}
/* ------------------ end Mat arithmetic operators ------------------*/

/*
 * Input-Output, comparison
 */
// input operator
template <size_t R, size_t C, class Num>
std::istream& operator>>(std::istream &is, Mat<R, C, Num> &M) {
    for (auto &m: M)
        if (!(is >> m)) break;
    return is;
}

// output operator
template <size_t R, size_t C, class Num>
std::ostream& operator<<(std::ostream &os, const Mat<R, C, Num> &M) {
    os << "{\n";
    for (const auto &m: M)
        os << m << '\n';
    return os << '}';
}

// comparison
template <size_t R1, size_t C1, class Num, size_t R2, size_t C2, class Comp>
inline bool operator==(const Mat<R1,C1,Num> &M1, const Mat<R2,C2,Comp> &M2) {
    if (R1 != R2 || C1 != C2) return false;
    for (size_t i {0}; i < R1; ++i)
        if (M1[i] != M2[i]) return false;
    return true;
}

template <size_t R1, size_t C1, class Num, size_t R2, size_t C2, class Comp>
inline bool operator!=(const Mat<R1,C1,Num> &M1, const Mat<R2,C2,Comp> &M2) {
    return !(M1 == M2);
}
/* --- end Input-Output, comparison --- */

/*
 * Additional methods and functions
 */
// fill Mat with a value
template <size_t R, size_t C, class Num>
inline void Mat<R, C, Num>::fill(const Num& val) {
    //std::fill(std::begin(V_), std::end(V_), val);
    for (auto &a: M_)
        a.fill(val);
}

// fill Mat row with a Vec
template <size_t R, size_t C, class Num>
inline void Mat<R, C, Num>::fill_row(const size_type i, const Vec<C, Num>& v) {
    M_[i] = v;
}

// fill Mat column with a Vec
template <size_t R, size_t C, class Num>
inline void Mat<R, C, Num>::fill_col(const size_type j, const Vec<R, Num>& v) {
    for (size_type i {0}; i < R; ++i)
        M_[i][j] = v[i];
}

// fill Mat rows with a Vec
template <size_t R, size_t C, class Num>
inline void Mat<R, C, Num>::fill_each_row(const Vec<C, Num> &v) {
    for (auto &m: M_)
        m = v;
}

// fill Mat columns with a Vec
template <size_t R, size_t C, class Num>
inline void Mat<R, C, Num>::fill_each_col(const Vec<R, Num> &v) {
    for (size_type j {0}; j < C; ++j)
        fill_col(j, v);
}

// transposition
template <size_t R, size_t C, class Num>
const Mat<C, R, Num> Mat<R, C, Num>::transpose() const {
    Mat<C, R, Num> M;
    for (auto i = C; i--; M[i] = this->col(i)) { }
    return M;
}

// identity matrix
template <size_t N>
inline Mat<N, N, int> eye() {
    Mat<N, N, int> M;
    for (auto i = N; i--; M[i][i] = 1) { }
    return M;
}

// matrix minor
template <size_t R, size_t C, class Num>
inline Mat<R - 1, C - 1, Num> mat_minor(const Mat<R, C, Num> &M,
        const size_t row_idx, const size_t col_idx) {
    Mat<R - 1, C - 1, Num> res;
    for (size_t i {0}; i < R - 1; ++i)
        for (size_t j {0}; j < C - 1; ++j)
            res[i][j] = M[i < row_idx ? i : i + 1][j < col_idx ? j : j + 1];
    return res;
}

/*
 * Various methods for determinant calculation
 */
// matrix determinant for 2x2 matrix
template <class Num>
constexpr Num det2(const Mat<2, 2, Num> &M) {
    return M[0][0] * M[1][1] - M[0][1] * M[1][0];
}

// matrix determinant for 3x3 matrix
template <class Num>
inline Num det3(const Mat<3, 3, Num> &M) {
    Num res {};
    for (size_t i {0}; i < 3; i += 2)
        res += M[i][0] * det2(mat_minor(M, i, 0));
    for (size_t i {1}; i < 3; i += 2)
        res -= M[i][0] * det2(mat_minor(M, i, 0));
    return res;
}

// matrix determinant for 4x4 matrix
template <class Num>
inline Num det4(const Mat<4, 4, Num> &M) {
    Num res {};
    for (size_t i {0}; i < 4; i += 2)
        res += M[i][0] * det3(mat_minor(M, i, 0));
    for (size_t i {1}; i < 4; i += 2)
        res -= M[i][0] * det3(mat_minor(M, i, 0));
    return res;
}

// Gauss method for determinant
template <size_t N, class Num>
inline Num det_gauss(const Mat<N, N, Num> &M) {
    Mat<N, N, double> X = Mat<N, N, double>(M);
    double d = 1;
    int num_repl {0};
    for (size_t i {0}; i < N - 1; ++i) {
        size_t max_idx = i;
        double max_val = std::abs(X[i][i]);
        for (size_t j {i + 1}; j < N; ++j) {
            const double val = std::abs(X[j][i]);
            if (val > max_val) {
                max_idx = j;
                max_val = val;
            }
        }

        if (max_idx > i) { // need to swap rows
            std::swap(X[i], X[max_idx]);
            ++num_repl;
        } else if (max_val == 0.0) { // check division by zero
            return max_val;
        }

        const double val = X[i][i];
        d *= val;

        for (size_t j {i + 1}; j < N; ++j) { // subtract the row
            const double q = X[j][i] / val;
            X[j][i] = 0;
            for (size_t k {i + 1}; k < N; ++k)
                X[j][k] -= X[i][k] * q;
        }
    }

    d *= X[N - 1][N - 1];
    return (num_repl & 1) ? Num(-d) : Num(d);
}

// Bareiss modification for the Gauss method for determinant (default)
template <size_t N, class Num>
inline Num det(const Mat<N, N, Num> &M) {
    Mat<N, N, Num> X {M};
    Num d = 1;
    size_t num_swap {0};
    for (size_t i {0}; i < N - 1; ++i) {
        auto max_idx = i;
        Num max_val = std::abs(X[i][i]);
        for (size_t j {i + 1}; j < N; ++j) {
            const Num val = std::abs(X[j][i]);
            if (val > max_val) {
                max_idx = j;
                max_val = val;
            }
        }

        if (max_idx > i) { // need to swap rows
            std::swap(X[i], X[max_idx]);
            ++num_swap;
        } else if (max_val == Num {}) { // check division by zero
            return max_val;
        }

        const Num val1 = X[i][i];
        for (size_t j {i + 1}; j < N; ++j) { // subtract the row
            const Num val2 = X[j][i];
            X[j][i] = Num {};
            for (size_t k {i + 1}; k < N; ++k)
                X[j][k] = (X[j][k] * val1 - X[i][k] * val2) / d;
        }
        d = val1;
    }

    return (num_swap & 1) ? -X[N - 1][N - 1] : X[N - 1][N - 1];
}
/* --- end various methods for determinant calculation --- */

// inverse transpose matrix
template <size_t N, class Num>
inline Mat<N, N, double> invert_transpose(const Mat<N, N, Num> &M) {
    Mat<N, N, double> res;
    for (size_t i {0}; i < N; ++i)
        for (size_t j {0}; j < N; ++j)
            res[i][j] = det(mat_minor(M, i, j)) * ((i + j) & 1 ? -1 : 1);
    return res / dot(res[0], M[0]);
}
/* --- end additional methods and functions --- */


} // Algebra_lib

#endif /* MAT_H */


