#include <algorithm>
#include "Model.h"

/*
 * ------------------ Vec_base implementation ------------------
 */
template <class T, size_t N>
Vec_base<T, N>::Vec_base(): a_() { }

template <class T, size_t N>
Vec_base<T, N>::Vec_base(const std::array<T,N> &a): a_(a) { }

template <class T, size_t N>
Vec_base<T, N>::Vec_base(const Vec_base &v): a_(v.a_) { }

template <class T, size_t N>
std::ostream& operator<<(std::ostream &os, const Vec_base<T, N> &v) {
    os << "{ ";
    for (size_t i {0}; i < N; ++i)
        os << v[i] << ' ';
    return os << '}';
}

/*
 * ------------------ Vertex implementation ------------------
 */
Vertex::Vertex(): Vec_base {} { }

Vertex::Vertex(const double a, const double b, const double c):
    Vec_base {std::array<double, 3> {a, b, c}} {
    }

Vertex::Vertex(const Vec_base<double, 3> &v): Vec_base {v} { }

Vertex::Vertex(const Vec_base<int, 3> &v): Vec_base {} {
    for (int i {0}; i < 3; ++i)
        a_[i] = v[i];
}

std::istream& operator>>(std::istream &is, Vertex &v) {
    double a, b, c;
    is >> a >> b >> c;
    if (is) v = Vertex {a, b, c};
    return is;
}

std::ostream& operator<<(std::ostream &os, const Vertex &v) {
    return os << (Vec_base<double, 3>&)v;
}

/*
 * ------------------ Normal implementation ------------------
 */
Normal::Normal(): Vertex() { }

Normal::Normal(const double a, const double b, const double c):
    Vertex {a, b, c} {
    }

Normal::Normal(const Vec_base<double, 3> &v): Vertex {v} { }

double Normal::norm() const {
    double d {0.0};
    for (const auto i: a_)
        d += i * i;
    return sqrt(d);
}

void Normal::normalize(const double p) {
    const double n {p / norm()};
    for (auto &i: a_)
        i *= n;
}

/*
 * ------------------ Point3 implementation ------------------
 */
Point3::Point3(): Vec_base {} { }

Point3::Point3(const int a, const int b, const int c):
    Vec_base {std::array<int, 3> {a, b, c}} {
    }

Point3::Point3(const double a, const double b, const double c):
    Vec_base {} {
        a_[0] = round(a); a_[1] = round(b); a_[2] = round(c);
    }

Point3::Point3(const Vec_base<int, 3> &v): Vec_base {v} { }

Point3::Point3(const Vec_base<double, 3> &v): Vec_base {} {
    for (int i {0}; i < 3; ++i)
        a_[i] = round(v[i]);
}

std::istream& operator>>(std::istream &is, Point3 &p) {
    int a, b, c;
    char ch;
    is >> a >> ch >> b >> ch >> c;
    if (is) p = Point3 {--a, --b, --c};
    return is;
}


std::ostream& operator<<(std::ostream &os, const Point3 &p) {
    return os << (Vec_base<int, 3>&)p;
}

/*
 * ------------------ Facet implementation ------------------
 */
Facet::Facet(): f_() { }

Facet::Facet(const Point3 &p1, const Point3 &p2, const Point3 &p3):
    f_(std::array<Point3, 3> {p1, p2, p3}) {
}

std::istream& operator>>(std::istream &is, Facet &f) {
    Point3 p1, p2, p3;
    is >> p1 >> p2 >> p3;
    if (is) f = Facet {p1, p2, p3};
    return is;
}

/*
 * ------------------ Matrix implementation ------------------
 */
Matrix::Matrix(const int r, const int c, const double d):
    m_{ std::vector<std::vector<double>> (r, std::vector<double>(c, d))} {
    }

Matrix  Matrix::identity(const int n) {
    Matrix E(n, n);
    for (int i {0}; i < n; ++i)
        E[i][i] = 1;
    return E;
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
    //static_assert(nr == nc, "Matrix should be square");
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

/*
 * ------------------ Model implementation ------------------
 */
Model::Model(const std::string &fn): verts_{}, faces_{}, norms_{}, texverts_{} {
    std::ifstream ifs {fn};
    if (!ifs)
        throw std::runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);

    for (std::string s; ifs >> s;) {
        if (s == "v") {
            Vertex v; ifs >> v;
            verts_.push_back(v);
        }
        if (s == "f") {
            // reading the triples: (vertex / texvertex / normal)
            Facet f; ifs >> f;
            faces_.push_back(f);
        }
        if (s == "vn") {
            // reading vertex normals
            Normal nv; ifs >> nv;
            norms_.push_back(nv);
        }
        if (s == "vt") {
            // reading texture vertices
            Vertex texv; ifs >> texv;
            texverts_.push_back(texv);
        }
        if (ifs.fail()) {
            ifs.unget();
            ifs.clear(std::ios_base::failbit);
        }
    }
}

const Vertex Model::texvertex(const int iface, const int ivert) const {
    const int idx {faces_[iface][ivert][1]};
    return Vertex {texverts_[idx][0], texverts_[idx][1], texverts_[idx][2]};
}

const Normal Model::normal(const int iface, const int ivert) const {
    Normal n {norms_[faces_[iface][ivert][2]]};
    n.normalize();
    return n;
}

void rasterize(Point &p1, Point &p2, std::vector<int> &ybuf, PPM_Image &img,
        const PPM_Color &c) {
    if (p1.x() > p2.x())
        std::swap(p1, p2);
    const int x1 {p1.x()}, x2 {p2.x()}, y1 {p1.y()}, y2 {p2.y()}, dy {y2 - y1};
    const double dx = x2 - x1;
    const int h {img.height()};
    const uint clr {c.color()};
    for (int x {x1}; x <= x2; ++x) {
        const int y = (x - x1) / dx * dy + y1;
        if (ybuf[x] < y) {
            ybuf[x] = y;
            for (int i {0}; i < h; ++i)
                img[x][i] = clr;
        }
    }
}

void triangle_buf(Point3 v1, Point3 v2, Point3 v3, std::vector<int> &z_buf,
        PPM_Image &img, const PPM_Color &c) {
    if (v1.y() == v2.y() && v2.y() == v3.y()) return;
    if (v1.y() > v2.y()) std::swap(v1, v2);
    if (v1.y() > v3.y()) std::swap(v1, v3);
    if (v2.y() > v3.y()) std::swap(v2, v3);
    int y1 {v1.y()}, y2 {v2.y()}, y3 {v3.y()};
    const int h {y3 - y1}, dy12 {y2- y1}, dy23 {y3 - y2};
    const bool is_y12 {y1 == y2};
    const Point3 v13 {v3 - v1}, v12 {v2 - v1}, v23 {v3 - v2};
    const uint clr {c.color()};
    for (int y {0}; y < h; ++y) {
        const bool up {y > dy12 || is_y12};
        const double a {static_cast<double>(y) / h};
        const double b {up ? static_cast<double>(y - dy12) / dy23 :
            static_cast<double>(y) / dy12};
        Point3 A {v1 + v13 * a}, B {up ? v2 + v23 * b : v1 + v12 * b};
        if (A.x() > B.x()) std::swap(A, B);
        const int xa {A.x()}, xb {B.x()}, dxab {xb - xa}, w {img.width()};
        const bool is_xab {xa == xb};
        const Vertex dAB = B - A, Av = A;
        for (int x {xa}; x <= xb; ++x) {
            const double phi {is_xab ? 1 : static_cast<double>(x - xa) / dxab};
            const Vertex P {Av + dAB * phi};
            const int idx = P.x() + P.y() * w;
            if (z_buf[idx] < P.z()) {
                z_buf[idx] = P.z();
                img[P.x()][P.y()] = clr;
            }
        }
    }
}

void triangle_tex(Point3 v1, Point3 v2, Point3 v3, Point3 uv1, Point3 uv2,
        Point3 uv3, std::vector<int> &z_buf, PPM_Image &img, PPM_Image &tex,
        const double br) {
    if (v1.y() == v2.y() && v2.y() == v3.y()) return;
    if (v1.y() > v2.y()) { std::swap(v1, v2); std::swap(uv1, uv2); }
    if (v1.y() > v3.y()) { std::swap(v1, v3); std::swap(uv1, uv3); }
    if (v2.y() > v3.y()) { std::swap(v2, v3); std::swap(uv2, uv3); }
    int y1 {v1.y()}, y2 {v2.y()}, y3 {v3.y()};
    const int h {y3 - y1}, dy12 {y2- y1}, dy23 {y3 - y2};
    const bool is_y12 {y1 == y2};
    const Point3 v13 {v3 - v1}, v12 {v2 - v1}, v23 {v3 - v2};
    const Point3 uv13 {uv3 - uv1}, uv12 {uv2 - uv1}, uv23 {uv3 - uv2};
    const int w {img.width()}, tex_h {tex.height()};
    for (int y {0}; y < h; ++y) {
        const bool up {y > dy12 || is_y12};
        const double a {static_cast<double>(y) / h};
        const double b {up ? static_cast<double>(y - dy12) / dy23 :
            static_cast<double>(y) / dy12};
        Point3 A {v1 + v13 * a}, B {up ? v2 + v23 * b : v1 + v12 * b};
        Point3 uvA {uv1 + uv13 * a}, uvB {up ? uv2 + uv23 * b : uv1 + uv12 * b};
        if (A.x() > B.x()) { std::swap(A, B); std::swap(uvA, uvB); }
        const int xa {A.x()}, xb {B.x()}, dxab {xb - xa};
        const bool is_xab {xa == xb};
        const Vertex dAB = B - A, Av = A, duvAB = uvB - uvA, uvAv = uvA;
        for (int x {xa}; x <= xb; ++x) {
            const double phi {is_xab ? 1 : static_cast<double>(x - xa) / dxab};
            const Vertex P {Av + dAB * phi};
            const Point3 uvP {uvAv + duvAB * phi};
            const int idx = P.x() + P.y() * w;
            if (z_buf[idx] < P.z()) {
                z_buf[idx] = P.z();
                PPM_Color clr {tex.color(uvP.x(), tex_h - uvP.y())};
                img.set_color(P.x(), P.y(), PPM_Color ((clr.red() * br),
                            (clr.green() * br), (clr.blue() * br)));
            }
        }
    }
}

void triangle_ref(Point3 v1, Point3 v2, Point3 v3, double iy1, double iy2,
        double iy3, std::vector<int> &z_buf, PPM_Image &img) {
    if (v1.y() == v2.y() && v2.y() == v3.y()) return;
    if (v1.y() > v2.y()) { std::swap(v1, v2); std::swap(iy1, iy2); }
    if (v1.y() > v3.y()) { std::swap(v1, v3); std::swap(iy1, iy3); }
    if (v2.y() > v3.y()) { std::swap(v2, v3); std::swap(iy2, iy3); }
    int y1 {v1.y()}, y2 {v2.y()}, y3 {v3.y()};
    const int h {y3 - y1}, dy12 {y2- y1}, dy23 {y3 - y2};
    const bool is_y12 {y1 == y2};
    const Point3 v13 {v3 - v1}, v12 {v2 - v1}, v23 {v3 - v2};
    const int img_w {img.width()}, img_h {img.height()};
    const double iy12 {iy2 - iy1}, iy13 {iy3 - iy1}, iy23 {iy3 - iy2};
    for (int y {0}; y < h; ++y) {
        const bool up {y > dy12 || is_y12};
        const double a {static_cast<double>(y) / h};
        const double b {up ? static_cast<double>(y - dy12) / dy23 :
            static_cast<double>(y) / dy12};
        Point3 A {v1 + v13 * a}, B {up ? v2 + v23 * b : v1 + v12 * b};
        double iyA {iy1 + iy13 * a}, iyB {up ? iy2 + iy23 * b : iy1 + iy12 * b};
        if (A.x() > B.x()) { std::swap(A, B); std::swap(iyA, iyB); }
        const int xa {A.x()}, xb {B.x()}, dxab {xb - xa};
        const bool is_xab {xa == xb};
        const Vertex dAB = B - A, Av = A;
        const double iyAB {iyB - iyA};
        for (int x {xa}; x <= xb; ++x) {
            const double phi {is_xab ? 1 : static_cast<double>(x - xa) / dxab};
            const Point3 P {Av + dAB * phi};
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
 * ------------------ Matrix-Vertex operations ------------------
 */
Vertex m2v(const Matrix &m) {
    const double r {m[3][0]};
    return Vertex {m[0][0] / r, m[1][0] / r, m[2][0] / r};
}

Matrix v2m(const Vertex &v) {
    Matrix m(4, 1);
    m[0][0] = v.x(); m[1][0] = v.y(); m[2][0] = v.z(); m[3][0] = 1.0;
    return m;
}

Matrix viewport(const int xx, const int yy, const int w, const int h,
        const int d) {
    Matrix m {Matrix::identity(4)};
    const double half_w {w / 2.0}, half_h {h / 2.0}, half_d {d / 2.0};
    m[0][0] = half_w; m[1][1] = half_h; m[2][2] = half_d;
    m[0][3] = xx + half_w; m[1][3] = yy + half_h; m[2][3] = half_d;
    return m;
}

Matrix lookat(const Normal eye, const Normal cen, const Vertex up) {
    Normal z {eye - cen}; z.normalize();
    Normal x {up ^ z}; x.normalize();
    Normal y {z ^ x}; y.normalize();
    Matrix Minv {Matrix::identity(4)};
    Matrix Tr {Matrix::identity(4)};
    for (int i {0}; i < 3; ++i) {
        Minv[0][i] = x[i]; Minv[1][i] = y[i]; Minv[2][i] = z[i];
        //Minv[i][3] = -cen[i];
        Tr[i][3] = -cen[i];
    }
    //return Minv;
    return Minv * Tr;
}

Matrix translation(const Vertex &v) {
    Matrix m {Matrix::identity(4)};
    m[0][3] = v.x(); m[1][3] = v.y(); m[2][3] = v.z();
    return m;
}

Matrix zoom(const double f) {
    Matrix m {Matrix::identity(4)};
    m[0][0] = m[1][1] = m[2][2] = f;
    return m;
}

Matrix rotation_x(const double cos_a, const double sin_a) {
    Matrix m {Matrix::identity(4)};
    m[1][1] = m[2][2] = cos_a; m[1][2] = -sin_a; m[2][1] = sin_a;
    return m;
}

Matrix rotation_y(const double cos_a, const double sin_a) {
    Matrix m {Matrix::identity(4)};
    m[0][0] = m[2][2] = cos_a; m[0][2] = sin_a; m[2][0] = -sin_a;
    return m;
}

Matrix rotation_z(const double cos_a, const double sin_a) {
    Matrix m {Matrix::identity(4)};
    m[0][0] = m[1][1] = cos_a; m[0][1] = -sin_a; m[1][0] = sin_a;
    return m;
}

