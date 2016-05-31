/*
 * Set of codes adapted from https://github.com/ssloy/tinyrenderer/wiki/
 * The model used only for learning/educational/training purposes and is the
 * same as in the original source.
 *
 */

#include "Model.h"
#include "Vec.h"
#include "Mat.h"
#include "Shader.h"
#include <iostream>
#include <algorithm>

using Vec3i = Vec<3, int>;
using Vec3d = Vec<3, double>;
using Mat4d = Mat<4, 4, double>;

class Shader: public IShader {
public:
    // triangle uv coordinates: written by the vertex shader, read by the
    // fragment shader
    Mat<2, 3, double> vary_uv;
    // triangle coordinates (screen space), written by VS, read by FS
    Mat<4, 3, double> vary_tri;

    Shader(): vary_uv{}, vary_tri{} { }

    ~Shader() = default;

    virtual Vec<4, double> vertex(const Model &m, const Mat4d &Proj,
            const Mat4d &ModelView, const int iface, const int ivert) {
        vary_uv.fill_col(ivert, resize<2>(m.texvertex(iface, ivert)));
        auto gl_vert = Proj * ModelView * resize<4>(m.vertex(iface, ivert));
        vary_tri.fill_col(ivert, gl_vert);
        return gl_vert;
    }

    virtual bool fragment(const PPM_Image &tex, const Vec<3, double> &bar,
            PPM_Color &C) {
        auto uv = vary_uv * bar;
        const auto h = tex.height();
        C = tex.color(uv[0] * tex.width(), h - uv[1] * h);
        return false;
    }
};

using Vec2i = Vec<2, int>;

Vec<3, double> baryc(const Vec2i &p1, const Vec2i &p2, const Vec2i &p3,
        const Vec2i &p) {
    const int x1 {p1.x()}, y1 {p1.y()}, x2 {p2.x()}, y2 {p2.y()};
    const int x3 {p3.x()}, y3 {p3.y()}, x {p.x()}, y {p.y()};
    const int dx13 {x1 - x3}, dx32 {x3 - x2}, dy13 {y1 - y3}, dy23 {y2 - y3};
    const double z = dy23 * dx13 + dx32 * dy13;
    if (std::abs(z) < 0.5) return {-1, 1, 1};
    const int dx3 {x - x3}, dy3 {y - y3};
    const double lam1 {(dy23 * dx3 + dx32 * dy3) / z};
    const double lam2 {(dx13 * dy3 - dy13 * dx3 ) / z};
    return {1 - lam1 - lam2, lam1, lam2};
}

void triangle_shader(const Mat<4, 3, double> &clipc, IShader &shader,
        const Mat4d &Viewport, PPM_Image &I, const PPM_Image &tex,
        std::vector<int> &zbuf) {
    Mat<3, 4, double> pts {(Viewport * clipc).transpose()};
    Mat<3, 2, double> pts2;
    for (int i {3}; i--; pts2[i] = pts[i] / pts[i][3]) { }
    const int w = I.width() - 1, h = I.height() - 1;
    using namespace std;
    auto xmin = max(min({pts2[0][0], pts2[1][0], pts2[2][0], double(w)}), 0.0);
    auto xmax = min(max({pts2[0][0], pts2[1][0], pts2[2][0], 0.0}), double(w));
    auto ymin = max(min({pts2[0][1], pts2[1][1], pts2[2][1], double(h)}), 0.0);
    auto ymax = min(max({pts2[0][1], pts2[1][1], pts2[2][1], 0.0}), double(h));
    Vec2i P;
    PPM_Color C;

    for (P.x() = xmin; P.x() <= xmax; ++P.x()) {
        for (P.y() = ymin; P.y() <= ymax; ++P.y()) {
            Vec3d bc_screen = baryc(pts2[0], pts2[1], pts2[2], P);
            Vec3d bc_clip = Vec3d{bc_screen.x() / pts[0][3],
                bc_screen.y() / pts[1][3], bc_screen.z() / pts[2][3]};
            bc_clip /= (bc_clip.x() + bc_clip.y() + bc_clip.z());
            double frag_dep = clipc[2] * bc_clip;
            if (bc_screen.x() < 0 || bc_screen.y() < 0 || bc_screen.z() < 0 ||
                    zbuf[P.x() + P.y() * w] > frag_dep) continue;
            if (!shader.fragment(tex, bc_clip, C)) {
                zbuf[P.x() + P.y() * w] = frag_dep;
                I[P.x()][h - P.y()] = C.color();
            }
        }
    }
}

void test_model() {
    using namespace std;
    const Model m {"../obj/african_head.obj"};
    cout << "Verts: " << m.num_vertices() << ", v[1]: " << m.vertex(1) << '\n';
    cout << "Norms: " << m.num_normals() << ", n[1]: " << m.normal(1,1) << '\n';
    cout << "Texture vertices: " << m.num_texvertices() <<
        ", texv[1][1]: " << m.texvertex(1, 1) << '\n';
    cout << "Faces: " << m.num_faces() << ", f[2]: " << m.face(2) << '\n';
    Vec3d v {m.normal(1, 2)}, v2 {m.normal(2, 1)};
    cout << v + v2 << endl;
}

void test_proj() {
    const Model m {"../obj/african_head.obj"};
    PPM_Image tex {"../obj/african_head_diffuse.ppm"};

    constexpr int w {800}, h {800}, d {255};
    PPM_Image img {w, h};

    Mat4d Viewport {viewport(w >> 3, h >> 3, (w >> 2) * 3, (h >> 2) * 3, d)};

    Vec3d L_dir {1, 1, 1};
    const Vec3d Eye {1, 1, 3}, Center {0, 0, 0}, Up {0, 1, 0};
    Mat4d ModelView {lookat(Eye, Center, Vec3d {0, 1.0, 0})};
    Mat4d Proj {projection(-1.0 / (Eye - Center).norm())};

    L_dir = Proj * ModelView * resize<4>(L_dir);
    L_dir.normalize();

    std::vector<int> zbuf(w * h, 0);
    Shader shader;
    for (size_t i {0}; i < m.num_faces(); ++i) {
        for (int j {0}; j < 3; ++j) {
            shader.vertex(m, Proj, ModelView, i, j);
        }
        triangle_shader(shader.vary_tri, shader, Viewport, img, tex, zbuf);
    }
    img.write_to("output.ppm");
}

void test_camera() {
    using namespace std;
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800}, d {255};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};

    const Vec3d light_dir {Vec3d{1, -1, 1}.normalize()}, Eye {1, 1, 3},
          center {0, 0, 0};
    const Mat4d ModelView {lookat(Eye, center, Vec3d{0, 1, 0})};
    const Mat4d VP {viewport(w >> 3, h >> 3, (w >> 2) * 3,
            (h >> 2) * 3, d)};
    Mat4d Proj = eye<4>();
    Proj[3][2] = -1.0 / (Eye - center).norm();
    const Mat4d Z {VP * Proj * ModelView};
    //cout << ModelView << Proj << VP << Z;
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Vec3i f {m.face(i)};
        // world coordinates
        const array<Vec3d, 3> wc {m.vertex(f[0]), m.vertex(f[1]),
            m.vertex(f[2])};
        // screen coordinates
        array<Vec3i, 3> sc;
        // light intensity (kind of brightness)
        array<double, 3> br;
        for (int j {0}; j < 3; ++j) {
            const auto vtemp = Z * resize<4>(wc[j]);
            sc[j] = vtemp / vtemp[3] + 0.5;
            sc[j].y() = h - sc[j].y(); // flip upside down
            br[j] = m.normal(i, j) * light_dir;
        }
        triangle_ref(sc[0], sc[1], sc[2], br[0], br[1], br[2], zbuf, img);
    }
    img.write_to("gouraud.ppm");

    PPM_Image zbimg {w, h};
    for (int i {0}; i < w; ++i)
        for (int j {0}; j < h; ++j)
            zbimg.set_color(i, j, zbuf[i + j * w]);
    zbimg.write_to("zbuffer.ppm");
}

void test_bary() {
    constexpr int w {600}, h {400};
    PPM_Image I {w, h};
    const Point p1 {-10, 10}, p2 {400, 100}, p3 {100, 550};
    Triangle {p1, p2, p3}.fill_bary(I, {128, 240, 75});
    //triangle_bar(p1, p2, p3, I, {128, 240, 75});
    I.write_to("output.ppm");
}

int main() {

    //test_model();
    //test_camera();
    test_proj();

    return 0;
}

