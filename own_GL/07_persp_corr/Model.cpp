#include "Model.h"

using Vec3i = Vec<3, int>;
using Vec3d = Vec<3, double>;
/*
 * ------------------ Facet implementation ------------------
 */
Facet::Facet(): f_() { }

Facet::Facet(const Vec3i &v1, const Vec3i &v2, const Vec3i &v3):
    f_(std::array<Vec3i, 3> {v1, v2, v3}) {
}

std::istream& operator>>(std::istream &is, Facet &f) {
    std::array<Vec3i, 3> vf;
    f = Facet();
    for (int i = 0; i < 3; ++i) {
        int a, b, c; char ch;
        if (is >> a >> ch >> b >> ch >> c)
            vf[i] = Vec3i{--a, --b, --c};
    }
    if (is) f = Facet{vf[0], vf[1], vf[2]};
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
            Vec3d v; ifs >> v;
            verts_.push_back(v);
        }
        if (s == "vn") {
            // reading vertex normals
            Vec3d nv; ifs >> nv;
            norms_.push_back(nv);
        }
        if (s == "vt") {
            // reading texture vertices
            Vec3d texv; ifs >> texv;
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

const Vec3d Model::vertex(const int iface, const int ivert) const {
    return verts_[faces_[iface][ivert][0]];
}

const Vec3d Model::texvertex(const int iface, const int ivert) const {
    const int idx {faces_[iface][ivert][1]};
    return Vec3d{texverts_[idx][0], texverts_[idx][1], texverts_[idx][2]};
}

const Vec3d Model::normal(const int iface, const int ivert) const {
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
        const Vec3d dAB = B - A, Av = A;
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

