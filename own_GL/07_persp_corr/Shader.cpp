#include "Shader.h"

using Vec2i = Vec<2, int>;
using Vec3d = Vec<3, double>;
using Vec4d = Vec<4, double>;
using Mat4d = Mat<4, 4, double>;

// barycentric coordinates: using vector cross product
Vec3d barycentric(const Vec2i &p1, const Vec2i &p2, const Vec2i &p3,
        const Vec2i &p) {
    Vec3d s[2];
    for (int i = 2; i--;) {
        s[i][0] = p3[i] - p1[i];
        s[i][1] = p2[i] - p1[i];
        s[i][2] = p1[i] - p[i];
    }
    Vec3d u = s[0] ^ s[1];
    if (std::abs(u.z()) > 0.01)
        return {1 - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z()};
    return {-1, 1, 1};
}

// barycentric coordinates: using direct calculations
Vec3d baryc(const Vec2i &p1, const Vec2i &p2, const Vec2i &p3, const Vec2i &p) {
    const int x1 {p1.x()}, dx31 {p3.x() - x1}, dx21 {p2.x() - x1};
    const int y1 {p1.y()}, dy31 {p3.y() - y1}, dy21 {p2.y() - y1};
    // the only double var here: contains int value but makes return double
    const double z = dx31 * dy21 - dx21 * dy31;
    if (std::abs(z) < 0.5)
        return {-1, 1, 1};
    const int dx1 {x1 - p.x()}, dy1 {y1 - p.y()};
    const int lam1 {dx21 * dy1 - dy21 * dx1}, lam2 {dy31 * dx1 - dx31 * dy1};
    return {1 - (lam1 + lam2) / z, lam2 / z, lam1 / z};
}

// draw triangle using own shaders
void triangle_shader(const Mat<3, 4, double> &pts, IShader &shader,
        PPM_Image &I, const PPM_Image &tex, std::vector<int> &zbuf) {
    Mat<3, 2, double> pts2;
    for (int i = 0; i < 3; ++i)
        pts2[i] = pts[i] / pts[i][3];
    const int img_w = I.width() - 1, img_h = I.height() - 1;
    auto xmin = std::max(std::min({pts2[0][0], pts2[1][0], pts2[2][0],
                double(img_w)}), 0.0);
    auto xmax = std::min(std::max({pts2[0][0], pts2[1][0], pts2[2][0],
                0.0}), double(img_w));
    auto ymin = std::max(std::min({pts2[0][1], pts2[1][1], pts2[2][1],
                double(img_h)}), 0.0);
    auto ymax = std::min(std::max({pts2[0][1], pts2[1][1], pts2[2][1],
                0.0}), double(img_h));

    for (int x = xmin; x <= xmax; ++x) {
        for (int y = ymin; y <= ymax; ++y) {
            const Vec3d bc = baryc(pts2[0], pts2[1], pts2[2], Vec2i{x, y});
            if (bc.x() < 0 || bc.y() < 0 || bc.z() < 0)
                continue;
            const double z {pts.col(2) * bc}, w {pts.col(3) * bc};
            const int frag_dep {std::max(0, std::min(255, int(z / w + 0.5)))};
            const int idx {x + y * img_w};
            if (zbuf[idx] < frag_dep) {
                PPM_Color C;
                if (!shader.fragment(tex, bc, C)) {
                    zbuf[idx] = frag_dep;
                    I[x][img_h - y] = C.color();
                }
            }
        }
    }

}

