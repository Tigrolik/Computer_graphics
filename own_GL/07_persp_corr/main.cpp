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
using Vec4d = Vec<4, double>;
using Mat4d = Mat<4, 4, double>;

void test_proj() {
    const Model m {"../obj/african_head.obj"};
    const PPM_Image tex {"../obj/african_head_diffuse.ppm"};

    constexpr int w {800}, h {800}, d {255};
    PPM_Image img {w, h};

    Vec3d L_dir {1, 1, 1};
    const Vec3d Eye {1, 1, 3}, Center {0, 0, 0}, Up {0, 1, 0};
    const Mat4d Viewport {viewport(w >> 3, h >> 3, (w >> 2) * 3,
            (h >> 2) * 3, d)};
    const Mat4d ModelView {lookat(Eye, Center, Up)};
    const Mat4d Proj {projection(-1.0 / (Eye - Center).norm())};

    //L_dir = Proj * ModelView * resize<4>(L_dir);
    L_dir = L_dir.normalize();

    std::vector<int> zbuf(w * h, 0);
    Tex_shader shader;
    //Gouraud_shader shader;
    for (size_t i {0}; i < m.num_faces(); ++i) {
        Mat<3, 4, double> sc_coords;
        for (int j {0}; j < 3; ++j) {
            sc_coords[j] = shader.vertex(m, Viewport, Proj, ModelView, L_dir,
                    i, j);
        }
        triangle_shader(sc_coords, shader, img, tex, zbuf);
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
    //cout << mat_minor(Z, 0, 0) << '\n';
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

int main() {

    test_camera();
    //test_proj();

    return 0;
}

