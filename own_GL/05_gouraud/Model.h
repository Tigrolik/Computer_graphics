/*
 * Current Model approach:
 * class Vec3 is a template class for arrays of fixed size.
 * The purpose is to create objects with 3 elements arrays of int or double
 *
 * Note: Vec3<int> and Vec3<double> declarations are given in the end of the
 * corresponding .cpp file
 *
 * With comparison to the original source I have the same names for Vec3 types,
 * however, the implementation details have subtle differences though (perhaps)
 * not signficant
 * Class Matrix is basically the same as in the original source. Functions
 * zoom(), lookat(), viewport() and others are implemented static alongside the
 * identity() function from the original source with aim to have them all in one
 * class and place
 */

#ifndef _MODEL_H_
#define _MODEL_H_

#include "PPM_Image.h"
#include "Geometry.h"
#include <fstream>
#include <array>
#include <vector>

template<class T>
constexpr T sqr(const T& t) {
    return t * t;
}

class Matrix; // forward declaration for class Matrix

/*
 * ------------------ Vec3 ------------------
 */
template <class T>
class Vec3 {
public:
    Vec3();
    Vec3(const std::array<T, 3>&);
    Vec3(const T&, const T&, const T& = T {});
    Vec3(const Vec3<double>&);
    Vec3(const Vec3<int>&);
    Vec3(const Matrix&);

    virtual ~Vec3() { }

    std::array<T, 3> values() const { return a_; }

    T& operator[](const int i) { return a_[i]; }
    const T& operator[](const int i) const { return a_[i]; }

    friend std::ostream& operator<<(std::ostream &os, const Vec3 &v) {
        os << "{ ";
        for (int i {0}; i < 3; ++i)
            os << v[i] << ' ';
        return os << '}';
    }
    friend std::istream& operator>>(std::istream&, Vec3<double>&);
    friend std::istream& operator>>(std::istream&, Vec3<int>&);

    size_t size() const { return 3; }

    const T x() const { return a_[0]; }
    const T y() const { return a_[1]; }
    const T z() const { return a_[2]; }

    double norm() const { return sqrt(sqr(x()) + sqr(y()) + sqr(z())); }
    Vec3 normalize(const double = 1.0);
    const Vec3 normalize(const double = 1.0) const;

private:
    std::array<T, 3> a_;
};

using Vec3f = Vec3<double>;
using Vec3i = Vec3<int>;

template <class T, class U>
Vec3<typename std::common_type<T, U>::type> operator+(
        const Vec3<T> &lhs, const Vec3<U> &rhs) {
    std::array<typename std::common_type<T, U>::type, 3> a;
    for (size_t i {0}; i < 3; ++i)
        a[i] = lhs[i] + rhs[i];
    return a;
}

template <class T, class U>
Vec3<typename std::common_type<T, U>::type> operator-(
        const Vec3<T> &lhs, const Vec3<U> &rhs) {
    std::array<typename std::common_type<T, U>::type, 3> a;
    for (size_t i {0}; i < 3; ++i)
        a[i] = lhs[i] - rhs[i];
    return a;
}

// multiply an array by a value
template <class T, class U>
Vec3<typename std::common_type<T, U>::type> operator*(
        const Vec3<T> &lhs, const U &rhs) {
    std::array<typename std::common_type<T, U>::type, 3> a;
    for (size_t i {0}; i < 3; ++i)
        a[i] = lhs[i] * rhs;
    return a;
}

// here, operator * is dot product
template <class T, class U>
typename std::common_type<T, U>::type operator*(
        const Vec3<T> &lhs, const Vec3<U> &rhs) {
    typename std::common_type<T, U>::type res {};
    for (size_t i {0}; i < 3; ++i)
        res += lhs[i] * rhs[i];
    return res;
}

// in our implementation operator ^ means cross product
template <class T, class U>
Vec3<typename std::common_type<T, U>::type> operator^(const
        Vec3<T> &lhs, const Vec3<U> &rhs) {
    std::array<typename std::common_type<T, U>::type, 3> a {
        lhs[1] * rhs[2] - lhs[2] * rhs[1], // y * z - z * y
        lhs[2] * rhs[0] - lhs[0] * rhs[2], // z * x - x * z
        lhs[0] * rhs[1] - lhs[1] * rhs[0], // x * y - y * x
    };
    return a;
}

/*
 * ------------------ Matrix ------------------
 */
class Matrix {
public:
    Matrix(const int = 4, const int = 4, const double = 0.0);
    Matrix(const Vec3<double>&);

    size_t num_rows() const { return m_.size(); }
    size_t num_cols() const { return m_[0].size(); }

    std::vector<double>& operator[](const int i) { return m_[i]; }
    const std::vector<double>& operator[](const int i) const { return m_[i]; }
    Matrix operator*(const Matrix&);
    const Matrix operator*(const Matrix&) const;

    Matrix transpose();
    Matrix inverse();

    friend std::ostream& operator<<(std::ostream&, const Matrix&);

    static Matrix identity(const int = 4);
    static Matrix viewport(const int, const int, const int, const int,
            const int);
    static Matrix lookat(const Vec3f&, const Vec3f&, const Vec3f&);
    static Matrix translation(const Vec3f&);
    static Matrix zoom(const double);
    static Matrix rotation_x(const double, const double);
    static Matrix rotation_y(const double, const double);
    static Matrix rotation_z(const double, const double);

private:
    std::vector<std::vector<double>> m_;
};

/*
 * ------------------ Facet ------------------
 */
class Facet {
public:
    Facet();
    Facet(const Vec3i&, const Vec3i&, const Vec3i&);

    Vec3i& operator[](const int i) { return f_[i]; }
    const Vec3i& operator[](const int i) const { return f_[i]; }

    friend std::istream& operator>>(std::istream&, Facet&);

private:
    std::array<Vec3i, 3> f_;
};

/*
 * ------------------ Model ------------------
 */
class Model {
public:
    Model(const std::string&);

    ~Model() = default;

    const Vec3f vertex(const int i) const { return verts_[i]; }
    const Vec3f texvertex(const int, const int) const;
    const Vec3f normal(const int, const int) const;
    const Vec3i face(const int) const;

    size_t num_vertices() const { return verts_.size(); }
    size_t num_faces() const { return faces_.size(); }
    size_t num_normals() const { return norms_.size(); }
    size_t num_texvertices() const { return texverts_.size(); }

private:
    std::vector<Vec3f> verts_;
    std::vector<Vec3f> norms_;
    std::vector<Vec3f> texverts_; // texture vertices
    std::vector<Facet> faces_;
};

/*
 * ------------------ Functions ------------------
 */
void triangle_ref(Vec3i, Vec3i, Vec3i, double, double, double,
        std::vector<int>&, PPM_Image&);

#endif

