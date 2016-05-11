#include "Model.h"

/*
 * ------------------ Vec3 implementation ------------------
 */
template <class T>
Vec3<T>::Vec3(): a_() { }

template <class T>
Vec3<T>::Vec3(const std::array<T, 3> &a): a_(a) { }

template <class T>
Vec3<T>::Vec3(const T &t1, const T &t2, const T &t3):
    Vec3 {std::array<T, 3> {t1, t2, t3}} {
}

template <class T>
Vec3<T>::Vec3(const Vec3<double> &v): a_() {
    if (std::is_integral<T>::value)
        for (int i {0}; i < 3; ++i)
            a_[i] = round(v[i]);
    else
        for (int i {0}; i < 3; ++i)
            a_[i] = v[i];
}

template <class T>
Vec3<T>::Vec3(const Vec3<int> &v): a_() {
    for (int i {0}; i < 3; ++i)
        a_[i] = v[i];
}

template <class T>
Vec3<T>::Vec3(const Matrix &m): Vec3<T> {Vec3<double>{m[0][0] / m[3][0],
    m[1][0] / m[3][0], m[2][0] / m[3][0]}} {
    }

std::istream& operator>>(std::istream &is, Vec3<double> &v) {
    double a, b, c;
    is >> a >> b >> c;
    if (is) v = Vec3<double>{a, b, c};
    return is;
}

std::istream& operator>>(std::istream &is, Vec3<int> &v) {
    int a, b, c;
    char ch;
    is >> a >> ch >> b >> ch >> c;
    if (is) v = Vec3<int>{--a, --b, --c};
    return is;
}

template <class T>
Vec3<T> Vec3<T>::normalize(const double p) {
    if (const double n {p / norm()})
        for (auto &i: a_)
            i *= n;
    return Vec3<T> {Vec3<double>{a_}};
}

template <class T>
const Vec3<T> Vec3<T>::normalize(const double p) const {
    if (const double n {p / norm()})
        return Vec3<T> {Vec3<double> {a_[0] * n, a_[1] * n, a_[2] * n}};
    else
        return Vec3<T>();
}

/*
 * ------------------ Matrix implementation ------------------
 */
Matrix::Matrix(const int r, const int c, const double d):
    m_{ std::vector<std::vector<double>> (r, std::vector<double>(c, d))} {
    }

Matrix::Matrix(const Vec3<double> &v): Matrix(4, 1, 1.0) {
    m_[0][0] = v.x(); m_[1][0] = v.y(); m_[2][0] = v.z();
}

Matrix Matrix::operator*(const Matrix &m) {
    const size_t r1 {num_rows()}, c1 {num_cols()};
    const size_t r2 {m.num_rows()}, c2 {m.num_cols()};
    if (c1 != r2)
        return Matrix(r1, c1);
    Matrix res(r1, c2);
    for (size_t r {0}; r < r1; ++r) {
        for (size_t c {0}; c < c2; ++c) {
            for (size_t i {0}; i < c1; ++i)
                res[r][c] += m_[r][i] * m[i][c];
        }
    }
    return res;
}

const Matrix Matrix::operator*(const Matrix &m) const {
    const size_t r1 {num_rows()}, c1 {num_cols()};
    const size_t r2 {m.num_rows()}, c2 {m.num_cols()};
    if (c1 != r2)
        return Matrix(r1, c1);
    Matrix res(r1, c2);
    for (size_t r {0}; r < r1; ++r) {
        for (size_t c {0}; c < c2; ++c) {
            for (size_t i {0}; i < c1; ++i)
                res[r][c] += m_[r][i] * m[i][c];
        }
    }
    return res;
}

Matrix Matrix::transpose() {
    const size_t nr {num_rows()}, nc {num_cols()};
    Matrix res(nc, nr);
    for (size_t r {0}; r < nr; ++r)
        for (size_t c {0}; c < nc; ++c)
            res[c][r] = m_[r][c];
    return res;
}

Matrix Matrix::inverse() {
    const size_t n {num_rows()};
    if (n != num_cols())
        return Matrix(n, 4);
    // augmenting the square matrix with the identity matrix of the same
    // dimensions: A => [AI]
    const size_t nc {n << 1};
    Matrix A(n, nc);
    for (size_t r {0}; r < n; ++r) {
        for (size_t c {0}; c < n; ++c)
            A[r][c] = m_[r][c];
        A[r][r + n] = 1;
    }
    const int ne = n - 1;
    // first pass
    for (int r {0}; r < ne; ++r) {
        // normalize the first row
        for (int c = nc - 1; c >= 0; --c)
            A[r][c] /= A[r][r];
        for (size_t k = r + 1; k < n; ++k) {
            const double coeff {A[k][r]};
            for (size_t j {0}; j < nc; ++j)
                A[k][j] -= A[r][j] * coeff;
        }
    }
    // normalize the last row
    for (int c = nc - 1; c >= ne; --c)
        A[ne][c] /= A[ne][ne];
    //second pass
    for (int r {ne}; r > 0; --r)
        for (int k = r - 1; k >= 0; --k) {
            const double coeff {A[k][r]};
            for (size_t j {0}; j < nc; ++j)
                A[k][j] -= A[r][j] * coeff;
        }
    // cut the identity matrix back
    Matrix res(n, n);
    for (size_t r {0}; r < n; ++r)
        for (size_t c {0}; c < n; ++c)
            res[r][c] = A[r][c + nc];
    return res;
}

std::ostream& operator<<(std::ostream &os, const Matrix &m) {
    os << "{\n";
    for (size_t r {0}; r < m.num_rows(); ++r) {
        os << m[r][0];
        for (size_t c {1}; c < m.num_cols(); ++c)
            os << ' ' << m[r][c];
        os << '\n';
    }
    return os << "}\n";
}

Matrix  Matrix::identity(const int n) {
    Matrix E(n, n);
    for (int i {0}; i < n; ++i)
        E[i][i] = 1;
    return E;
}

Matrix Matrix::viewport(const int xx, const int yy, const int w, const int h,
        const int d) {
    Matrix m {Matrix::identity()};
    const double half_w {w / 2.0}, half_h {h / 2.0}, half_d {d / 2.0};
    m[0][0] = half_w; m[1][1] = half_h; m[2][2] = half_d;
    m[0][3] = xx + half_w; m[1][3] = yy + half_h; m[2][3] = half_d;
    return m;
}

Matrix Matrix::lookat(const Vec3f &eye, const Vec3f &cen, const Vec3f &up) {
    Vec3f z {(eye - cen).normalize()};
    Vec3f x {(up ^ z).normalize()};
    Vec3f y {(z ^ x).normalize()};
    Matrix Minv {Matrix::identity()};
    Matrix Tr {Matrix::identity()};
    for (int i {0}; i < 3; ++i) {
        Minv[0][i] = x[i]; Minv[1][i] = y[i]; Minv[2][i] = z[i];
        Tr[i][3] = -cen[i];
    }
    return Minv * Tr;
}

Matrix Matrix::translation(const Vec3f &v) {
    Matrix m {Matrix::identity()};
    m[0][3] = v.x(); m[1][3] = v.y(); m[2][3] = v.z();
    return m;
}

Matrix Matrix::zoom(const double f) {
    Matrix m {Matrix::identity()};
    m[0][0] = m[1][1] = m[2][2] = f;
    return m;
}

Matrix Matrix::rotation_x(const double cos_a, const double sin_a) {
    Matrix m {Matrix::identity()};
    m[1][1] = m[2][2] = cos_a; m[1][2] = -sin_a; m[2][1] = sin_a;
    return m;
}

Matrix Matrix::rotation_y(const double cos_a, const double sin_a) {
    Matrix m {Matrix::identity()};
    m[0][0] = m[2][2] = cos_a; m[0][2] = sin_a; m[2][0] = -sin_a;
    return m;
}

Matrix Matrix::rotation_z(const double cos_a, const double sin_a) {
    Matrix m {Matrix::identity()};
    m[0][0] = m[1][1] = cos_a; m[0][1] = -sin_a; m[1][0] = sin_a;
    return m;
}

/*
 * ------------------ Facet implementation ------------------
 */
Facet::Facet(): f_() { }

Facet::Facet(const Vec3i &v1, const Vec3i &v2, const Vec3i &v3):
    f_(std::array<Vec3i, 3> {v1, v2, v3}) {
}

std::istream& operator>>(std::istream &is, Facet &f) {
    Vec3i v1, v2, v3;
    is >> v1 >> v2 >> v3;
    if (is) f = Facet {v1, v2, v3};
    return is;
}

/*
 * ------------------ Model implementation ------------------
 */
Model::Model(const std::string &fn): verts_{}, norms_{}, texverts_{}, faces_{} {
    std::ifstream ifs {fn};
    if (!ifs)
        throw std::runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);

    for (std::string s; ifs >> s;) {
        if (s == "v") {
            Vec3f v; ifs >> v;
            verts_.push_back(v);
        }
        if (s == "vn") {
            // reading vertex normals
            Vec3f nv; ifs >> nv;
            norms_.push_back(nv);
        }
        if (s == "vt") {
            // reading texture vertices
            Vec3f texv; ifs >> texv;
            texverts_.push_back(texv);
        }
        if (s == "f") {
            // reading the triples: (vertex / texvertex / normal)
            Facet f; ifs >> f;
            faces_.push_back(f);
        }
        if (ifs.fail()) {
            ifs.unget();
            ifs.clear(std::ios_base::failbit);
        }
    }
}

const Vec3f Model::texvertex(const int iface, const int ivert) const {
    const int idx {faces_[iface][ivert][1]};
    return {texverts_[idx][0], texverts_[idx][1], texverts_[idx][2]};
}

const Vec3f Model::normal(const int iface, const int ivert) const {
    return (norms_[faces_[iface][ivert][2]]).normalize();
}

const Vec3i Model::face(const int idx) const {
    return {faces_[idx][0][0], faces_[idx][1][0], faces_[idx][2][0]};
}

/*
 * ------------------ Functions ------------------
 */
void triangle_ref(Vec3i v1, Vec3i v2, Vec3i v3, double iy1, double iy2,
        double iy3, std::vector<int> &z_buf, PPM_Image &img) {
    if (v1.y() == v2.y() && v2.y() == v3.y()) return;
    if (v1.y() > v2.y()) { std::swap(v1, v2); std::swap(iy1, iy2); }
    if (v1.y() > v3.y()) { std::swap(v1, v3); std::swap(iy1, iy3); }
    if (v2.y() > v3.y()) { std::swap(v2, v3); std::swap(iy2, iy3); }
    int y1 {v1.y()}, y2 {v2.y()}, y3 {v3.y()};
    const int h {y3 - y1}, dy12 {y2- y1}, dy23 {y3 - y2};
    const bool is_y12 {y1 == y2};
    const Vec3i v13 {v3 - v1}, v12 {v2 - v1}, v23 {v3 - v2};
    const double iy12 {iy2 - iy1}, iy13 {iy3 - iy1}, iy23 {iy3 - iy2};
    const int img_w {img.width()}, img_h {img.height()};
    for (int y {0}; y < h; ++y) {
        const bool up {y > dy12 || is_y12};
        const double a {static_cast<double>(y) / h};
        const double b {up ? static_cast<double>(y - dy12) / dy23 :
            static_cast<double>(y) / dy12};
        Vec3i A {v1 + v13 * a}, B {up ? v2 + v23 * b : v1 + v12 * b};
        double iyA {iy1 + iy13 * a}, iyB {up ? iy2 + iy23 * b : iy1 + iy12 * b};
        if (A.x() > B.x()) { std::swap(A, B); std::swap(iyA, iyB); }
        const int xa {A.x()}, xb {B.x()}, dxab {xb - xa};
        const bool is_xab {xa == xb};
        const Vec3f dAB = B - A, Av = A;
        const double iyAB {iyB - iyA};
        for (int x {xa}; x <= xb; ++x) {
            const double phi {is_xab ? 1 : static_cast<double>(x - xa) / dxab};
            const Vec3i P {Av + dAB * phi};
            const int xp = P.x(), yp = P.y();
            if (xp >= img_w || yp >= img_h || xp < 0 || yp < 0) continue;
            const int idx {xp + yp * img_w};
            if (z_buf[idx] < P.z()) {
                z_buf[idx] = P.z();
                const double iyP {iyA + iyAB * phi};
                img.set_color(xp, yp, iyP > 1 ? 255 : iyP < 0 ? 0 : iyP * 255);
            }
        }
    }
}

/*
 * ------------------ Specific classes declarations ------------------
 */
template class Vec3<int>;
template class Vec3<double>;

