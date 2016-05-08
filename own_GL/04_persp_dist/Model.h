/*
 * Now Model uses another approach:
 * class Vec_base is a base class for arrays of fixed size (it is said that
 * std::arrays of small size (2-3 in our case) might be more efficient than
 * std::vectors, so I have decided to practice using them for a while).
 * The purpose is to create classes for 2-3 element arrays of int or double
 *
 * Note: without classes which explicitly define template parameters (like
 * Vertex with 3 doubles) the design would require to put explicit type
 * declarations in the class' .cpp file
 *
 * With comparison to the previous exercise and the original source I neither
 * use std::vectors nor Vec3f, Vec2i, but rather create classes Vertex, Normal
 * and Point3 as replacements. Again, only for practice purposes and not
 * because the original solutions is not viable.
 *
 * Additionally we introduce classes Facet and Matrix:
 *     Facet is used in Model for reading faces data and with the purpose of
 *     avoiding nested std::vectors and to have more or less uniform style for
 *     Model class fields;
 *     Matrix is basically the same as in the original source.
 */

#ifndef _MODEL_H_
#define _MODEL_H_

#include "PPM_Image.h"
#include "Geometry.h"
#include <fstream>
#include <array>
#include <vector>

template <class T, size_t N> class Vec_base; // forward declaration
template <class T, size_t N>
std::ostream& operator<<(std::ostream&, const Vec_base<T, N>&);

template <class T, size_t N>
class Vec_base {
//protected:
public:
    Vec_base();
    Vec_base(const std::array<T, N>&);
    Vec_base(const Vec_base&);

//public:
    virtual ~Vec_base() { }

    friend std::ostream& operator<< <> (std::ostream&, const Vec_base&);

    T operator[](const int i) { return a_[i]; }
    const T& operator[](const int i) const { return a_[i]; }

    size_t size() const { return N; }

protected:
    std::array<T, N> a_;
};

template <class T, class U, size_t N>
Vec_base<typename std::common_type<T, U>::type, N> operator+(
        const Vec_base<T, N> &lhs, const Vec_base<U, N> &rhs) {
    std::array<typename std::common_type<T, U>::type, N> a;
    for (size_t i {0}; i < N; ++i)
        a[i] = lhs[i] + rhs[i];
    return a;
}

template <class T, class U, size_t N>
Vec_base<typename std::common_type<T, U>::type, N> operator-(
        const Vec_base<T, N> &lhs, const Vec_base<U, N> &rhs) {
    std::array<typename std::common_type<T, U>::type, N> a;
    for (size_t i {0}; i < N; ++i)
        a[i] = lhs[i] - rhs[i];
    return a;
}

// multiply an array by a value
template <class T, class U, size_t N>
Vec_base<typename std::common_type<T, U>::type, N> operator*(
        const Vec_base<T, N> &lhs, const U &rhs) {
    std::array<typename std::common_type<T, U>::type, N> a;
    for (size_t i {0}; i < N; ++i)
        a[i] = lhs[i] * rhs;
    return a;
}

// here, operator * is dot product
template <class T, class U, size_t N>
typename std::common_type<T, U>::type operator*(
        const Vec_base<T, N> &lhs, const Vec_base<U, N> &rhs) {
    typename std::common_type<T, U>::type res {};
    for (size_t i {0}; i < N; ++i)
        res += lhs[i] * rhs[i];
    return res;
}

// in our implementation operator ^ means cross product
template <class T, class U>
Vec_base<typename std::common_type<T, U>::type, 3> operator^(const
        Vec_base<T, 3> &lhs, const Vec_base<U, 3> &rhs) {
    std::array<typename std::common_type<T, U>::type, 3> a {
        lhs[1] * rhs[2] - lhs[2] * rhs[1], // y * z - z * y
            lhs[2] * rhs[0] - lhs[0] * rhs[2], // z * x - x * z
            lhs[0] * rhs[1] - lhs[1] * rhs[0], // x * y - y * x
    };
    return a;
}


class Vertex: public Vec_base<double, 3> {
public:
    Vertex();
    Vertex(const double, const double, const double);
    Vertex(const Vec_base<double, 3>&);
    Vertex(const Vec_base<int, 3>&);

    friend std::istream& operator>>(std::istream&, Vertex&);

    double x() const { return a_[0]; }
    double y() const { return a_[1]; }
    double z() const { return a_[2]; }
};

/*
 * class Normal
 * Basically this class copies Vertex, the reason to have it is merely for a
 * name and with possible addition of specific methods
 */
class Normal: public Vertex {
public:
    Normal();
    Normal(const double, const double, const double);
    Normal(const Vec_base<double, 3>&);

    double norm() const;
    void normalize(const double = 1.0);
};

class Point3: public Vec_base<int, 3> {
public:
    Point3();
    Point3(const int, const int, const int = 0);
    Point3(const double, const double, const double = 0.0);
    Point3(const Vec_base<int, 3>&);
    Point3(const Vec_base<double, 3>&);

    friend std::istream& operator>>(std::istream&, Point3&);

    int x() const { return a_[0]; }
    int y() const { return a_[1]; }
    int z() const { return a_[2]; }
};

class Facet {
public:
    Facet();
    Facet(const Point3&, const Point3&, const Point3&);

    Point3 operator[](const int i) { return f_[i]; }
    const Point3& operator[](const int i) const { return f_[i]; }

    friend std::istream& operator>>(std::istream&, Facet&);

private:
    std::array<Point3, 3> f_;
};

class Matrix {
public:
    Matrix(const int = 4, const int = 4, const double = 0.0);

    static Matrix identity(const int);

    std::vector<double> &operator[](const int i) { return m_[i]; }
    const std::vector<double> &operator[](const int i) const { return m_[i]; }

    Matrix operator*(const Matrix&);
    const Matrix operator*(const Matrix&) const;

    size_t num_rows() const { return m_.size(); }
    size_t num_cols() const { return m_[0].size(); }

    friend std::ostream& operator<<(std::ostream&, const Matrix&);

    Matrix transpose();
    Matrix inverse();

private:
    std::vector<std::vector<double>> m_;
};

class Model {
public:
    Model(const std::string &);

    ~Model() = default;

    const Vertex vertex(const int i) const { return verts_[i]; }
    const Point3 face(const int idx) const {
        return {faces_[idx][0][0], faces_[idx][1][0], faces_[idx][2][0]};
    }
    const Vertex texvertex(const int, const int) const;
    //const Normal normal(const int i) const { return norms_[i]; }
    const Normal normal(const int, const int) const;

    size_t num_vertices() const { return verts_.size(); }
    size_t num_faces() const { return faces_.size(); }
    size_t num_normals() const { return norms_.size(); }
    size_t num_texvertices() const { return texverts_.size(); }

private:
    std::vector<Vertex> verts_;
    std::vector<Facet> faces_;
    std::vector<Normal> norms_;
    std::vector<Vertex> texverts_; // texture vertices
};

void rasterize(Point&, Point&, std::vector<int>&, PPM_Image&,
        const PPM_Color& = Color_name::white);

void triangle_buf(Point3, Point3, Point3, std::vector<int>&, PPM_Image&,
        const PPM_Color& = Color_name::white);

void triangle_tex(Point3, Point3, Point3, Point3, Point3, Point3,
        std::vector<int>&, PPM_Image&, PPM_Image&, const double = 1.0);

void triangle_ref(Point3, Point3, Point3, double, double, double,
        std::vector<int>&, PPM_Image&);

Vertex m2v(const Matrix&);

Matrix v2m(const Vertex&);

Matrix viewport(const int, const int, const int, const int, const int);

Matrix lookat(const Normal, const Normal, const Vertex);

Matrix translation(const Vertex&);

Matrix zoom(const double);

Matrix rotation_x(const double, const double);

Matrix rotation_y(const double, const double);

Matrix rotation_z(const double, const double);

#endif

