#include <stdexcept>
#include "Model.h"

using namespace std;

Vertex::Vertex(const double xx, const double yy, const double zz):
    v_{xx, yy, zz} {
    }

Model::Model(const std::string &fn): verts_{}, faces_{}, norms_{}, texverts_{} {
    std::ifstream ifs {fn};
    if (!ifs)
        throw std::runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);

    for (std::string s; ifs >> s;) {
        if (s == "v") {
            // read x, y, z coordinates of the vertices
            std::vector<double> v(3);
            for (auto &x: v)
                ifs >> x;
            verts_.push_back(v);
        }
        if (s == "f") {
            // reading the triples: (vertex / texvertex / normal)
            std::vector<std::vector<int>> v(3);
            for (auto &x: v) {
                int i; char c;
                ifs >> i; x.push_back(--i); ifs >> c;
                ifs >> i; x.push_back(--i); ifs >> c;
                ifs >> i; x.push_back(--i);
            }
            faces_.push_back(v);
        }
        if (s == "vn") {
            // reading vertex normals
            std::vector<double> v(3);
            for (auto &x: v)
                ifs >> x;
            norms_.push_back(v);
        }
        if (s == "vt") {
            // reading texture vertices (ignoring the third value)
            std::vector<double> v(2);
            for (auto &x: v)
                ifs >> x;
            texverts_.push_back(v);
            double d; ifs >> d;
        }
        if (ifs.fail()) {
            ifs.unget();
            ifs.clear(std::ios_base::failbit);
        }
    }
}

vector<int> Model::face(const int idx) const {
    vector<int> v(3);
    for (int i {0}; i < 3; ++i)
        v[i] = faces_[idx][i][0];
    return v;
}

vector<double> Model::texvertex(const int iface, const int nvert) const {
    const int idx {faces_[iface][nvert][1]};
    return vector<double> {texverts_[idx][0], texverts_[idx][1]};
}

vector<double> sum(const vector<double> &v1, const vector<double> &v2) {
    const auto n = v1.size();
    vector<double> w(n);
    if (n != v2.size())
        throw runtime_error("error: vectors sizes do not match");
    else
        for (size_t i {0}; i < n; ++i)
            w[i] = v1[i] + v2[i];
    return w;
}

vector<int> sum(const vector<int> &v1, const vector<int> &v2) {
    const auto n = v1.size();
    vector<int> w(n);
    if (n != v2.size())
        throw runtime_error("error: vectors sizes do not match");
    else
        for (size_t i {0}; i < n; ++i)
            w[i] = v1[i] + v2[i];
    return w;
}

vector<double> diff(const vector<double> &v1, const vector<double> &v2) {
    const auto n = v1.size();
    vector<double> w(n);
    if (n != v2.size())
        throw runtime_error("error: vectors sizes do not match");
    else
        for (size_t i {0}; i < n; ++i)
            w[i] = v1[i] - v2[i];
    return w;
}

vector<int> diff(const vector<int> &v1, const vector<int> &v2) {
    const auto n = v1.size();
    vector<int> w(n);
    if (n != v2.size())
        throw runtime_error("error: vectors sizes do not match");
    else
        for (size_t i {0}; i < n; ++i)
            w[i] = v1[i] - v2[i];
    return w;
}

vector<double> prod(const vector<int> &v, const double d) {
    const auto n = v.size();
    vector<double> w(n);
    for (size_t i {0}; i < n; ++i)
        w[i] = v[i] * d;
    return w;
}

vector<double> prod(const vector<double> &v, const double d) {
    const auto n = v.size();
    vector<double> w(n);
    for (size_t i {0}; i < n; ++i)
        w[i] = v[i] * d;
    return w;
}

double dot(const vector<double> &v1, const vector<double> &v2) {
    return inner_product(v1.begin(), v1.end(), v2.begin(), double{});
}

vector<double> cross(const vector<double> &v1, const vector<double> &v2) {
    const auto n = v1.size();
    if (n != v2.size() || n != 3) {
        throw runtime_error("error: vectors sizes do not match");
        return vector<double> {};
    }
    return vector<double> {
        v1[1] * v2[2] - v1[2] * v2[1], // y * z - z * y
            v1[2] * v2[0] - v1[0] * v2[2], // z * x - x * z
            v1[0] * v2[1] - v1[1] * v2[0], // x * y - y * x
    };
}

double norm(const vector<double> &v) {
    double d {0.0};
    for (const auto x: v)
        d += x * x;
    return sqrt(d);
}

void normalize(vector<double> &v, const double p) {
    const double n {p / norm(v)};
    for (auto &x: v)
        x *= n;
}

vector<int> round(const vector<double> &v) {
    const auto n = v.size();
    vector<int> w(n);
    for (size_t i {0}; i < n; ++i)
        w[i] = round(v[i]);
    return w;
}

vector<double> itod(const vector<int> &v) {
    return vector<double> {static_cast<double>(v[0]),
        static_cast<double>(v[1]), static_cast<double>(v[2])};
}

void rasterize(Point &p1, Point &p2, vector<int> &ybuf, PPM_Image &img,
        const PPM_Color &c) {
    if (p1.x() > p2.x())
        swap(p1, p2);
    const int x1 {p1.x()}, x2 {p2.x()}, y1 {p1.y()}, y2 {p2.y()}, dy {y2 - y1};
    const double dx {static_cast<double>(x2 - x1)};
    const int h {img.height()};
    for (int x {x1}; x <= x2; ++x) {
        const int y {static_cast<int>((x - x1) / dx * dy) + y1};
        if (ybuf[x] < y) {
            ybuf[x] = y;
            for (int i {0}; i < h; ++i)
                img.pixel_color(x, i, c);
        }
    }
}

void triangle_buf(vector<int> v1, vector<int> v2, vector<int> v3,
        vector<int> &zbuf, PPM_Image &img,
        const PPM_Color &c) {
    if (v1[1] == v2[1] && v2[1] == v3[1]) return;
    if (v1[1] > v2[1]) std::swap(v1, v2);
    if (v1[1] > v3[1]) std::swap(v1, v3);
    if (v2[1] > v3[1]) std::swap(v2, v3);
    int y1 {v1[1]}, y2 {v2[1]}, y3 {v3[1]};
    const int h {y3 - y1}, dy12 {y2- y1}, dy23 {y3 - y2};
    const bool is_y12 {y1 == y2};
    const vector<int> v13 {diff(v3,v1)}, v12 {diff(v2,v1)}, v23 {diff(v3,v2)};
    for (int y {0}; y < h; ++y) {
        const bool up {y > dy12 || is_y12};
        const double a {static_cast<double>(y) / h};
        const double b {up ? static_cast<double>(y - dy12) / dy23 :
            static_cast<double>(y) / dy12};
        vector<int> A {sum(v1, round(prod(v13, a)))};
        vector<int> B {up ? sum(v2, round(prod(v23, b))) :
            sum(v1, round(prod(v12, b)))};
        if (A[0] > B[0]) swap(A, B);
        const int xa {A[0]}, xb {B[0]}, dxab {xb - xa}, w {img.width()};
        const bool is_xab {xa == xb};
        const vector<double> dAB {itod(diff(B, A))}, Av {itod(A)};
        // fill the triangle
        for (int x {xa}; x <= xb; ++x) {
            const double phi {is_xab ? 1 : static_cast<double>(x - xa) / dxab};
            const vector<double> P {sum(Av, prod(dAB, phi))};
            const int idx {static_cast<int>(P[0] + P[1] * w)};
            if (zbuf[idx] < P[2]) {
                zbuf[idx] = P[2];
                img.pixel_color(P[0], P[1], c);
            }
        }
    }
}

void triangle_tex(vector<int> v1, vector<int> v2, vector<int> v3,
        vector<int> uv1, vector<int> uv2, vector<int> uv3, vector<int> &zbuf,
        PPM_Image &img, PPM_Image &tex, const double br) {
    if (v1[1] == v2[1] && v2[1] == v3[1]) return;
    if (v1[1] > v2[1]) {
        std::swap(v1, v2);
        std::swap(uv1, uv2);
    }
    if (v1[1] > v3[1]) {
        std::swap(v1, v3);
        std::swap(uv1, uv3);
    }
    if (v2[1] > v3[1]) {
        std::swap(v2, v3);
        std::swap(uv2, uv3);
    }
    int y1 {v1[1]}, y2 {v2[1]}, y3 {v3[1]};
    const int h {y3 - y1}, dy12 {y2- y1}, dy23 {y3 - y2};
    const bool is_y12 {y1 == y2};
    const vector<int> v13 {diff(v3,v1)}, v12 {diff(v2,v1)}, v23 {diff(v3,v2)};
    const vector<int> uv13 {diff(uv3,uv1)}, uv12 {diff(uv2,uv1)},
          uv23 {diff(uv3,uv2)};
    const int w {img.width()}, tex_h {tex.height()};
    for (int y {0}; y < h; ++y) {
        const bool up {y > dy12 || is_y12};
        const double a {static_cast<double>(y) / h};
        const double b {up ? static_cast<double>(y - dy12) / dy23 :
            static_cast<double>(y) / dy12};
        vector<int> A {sum(v1, round(prod(v13, a)))};
        vector<int> B {up ? sum(v2, round(prod(v23, b))) :
            sum(v1, round(prod(v12, b)))};
        vector<int> uvA {sum(uv1, round(prod(uv13, a)))};
        vector<int> uvB {up ? sum(uv2, round(prod(uv23, b))) :
            sum(uv1, round(prod(uv12, b)))};
        if (A[0] > B[0]){
            swap(A, B);
            swap(uvA, uvB);
        }
        const int xa {A[0]}, xb {B[0]}, dxab {xb - xa};
        const bool is_xab {xa == xb};
        const vector<double> dAB {itod(diff(B, A))}, Av {itod(A)};
        const vector<double> uvAv {itod(uvA)}, duvAB {itod(diff(uvB, uvA))};
        for (int x {xa}; x <= xb; ++x) {
            const double phi {is_xab ? 1 : static_cast<double>(x - xa) / dxab};
            const vector<double> P {sum(Av, prod(dAB, phi))};
            const vector<int> uvP {round(sum(uvAv, prod(duvAB, phi)))};
            const int idx {static_cast<int>(P[0] + P[1] * w)};
            if (zbuf[idx] < P[2]) {
                zbuf[idx] = P[2];
                PPM_Color clr {tex.get_color(uvP[0], tex_h - uvP[1])};
                img.pixel_color(P[0], P[1],
                        PPM_Color {static_cast<uchar>(clr.red() * br),
                        static_cast<uchar>(clr.green() * br),
                        static_cast<uchar>(clr.blue() * br)});
            }
        }
    }
}

