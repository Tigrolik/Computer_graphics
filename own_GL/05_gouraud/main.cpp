/*
 * Set of codes adapted from https://github.com/ssloy/tinyrenderer/wiki/
 * The model used only for learning/educational/training purposes and is the
 * same as in the original source.
 *
 * This exercise use Vec3 templated class with types int and double
 * (Vec3<double> with short name Vec3f and Vec3<int> with short name Vec3i) with
 * names similar to ones used in the original source. Using Vec3f and not Vec3d
 * for type double seems less confusing (in a sense of 3d as a reference to
 * three dimensions), however, might become a problem if we want to have a
 * short name for type float
 */

#include "Model.h"
#include <iostream>

using namespace std;

void test_init() {
    Vec3f vf1 {3.5, 4.2, 0.5};
    vf1[1] = vf1[1] + 3;
    cout << vf1.x() << ' ' << vf1.y() << ' ' << vf1.z() << endl;
    Vec3i vi1 (1.5, 2, 3);
    cout << vi1 << endl;
    Vec3f vf2 {vi1};
    cout << vf2 << endl;
    Vec3i vi2 {vf1};
    cout << vi2 << endl;
    cout << vi1 + vi2 << endl;
    cout << vi1 + vf1 << endl;
    cout << vf2 + vi2 << endl;
    Vec3<double> vd1 {2.07, 3.9023};
    cout << vd1 << endl;
    vd1.normalize();
    cout << vd1 << endl;
    Vec3f vf3 {vd1.normalize()};
    cout << vd1 << ' ' << vf3 << endl;
    cout << sqr(vd1.x()) +sqr(vd1.y()) << endl;
}

void test_matrix() {
    Vec3f vf1 {3.5, 4.2, 0.5};
    Matrix m1 {vf1};
    Vec3i vi1 (1.5, 2, 3);
    Matrix m2 {vi1};
    cout << m1 << m2;
    //cout << Matrix::identity() << Matrix::identity(3);
    Vec3f vf2 {m2};
    cout << vf2 << endl;
}

void test_model() {
    const Model m {"../obj/african_head.obj"};
    cout << "Verts: " << m.num_vertices() << ", v[1]: " << m.vertex(1) << endl;
    cout << "Norms: " << m.num_normals() << ", n[1]: " << m.normal(1,1) << endl;
    cout << "Texture vertices: " << m.num_texvertices() <<
        ", texv[1][1]: " << m.texvertex(1, 1) << endl;
    cout << "Faces: " << m.num_faces() << ", f[1]: " << m.face(1) << endl;
    Vec3f v {m.normal(1, 2)}, v2 {m.normal(2, 1)};
    cout << v + v2 << endl;
}

void test_camera() {
    const Model m {"../obj/african_head.obj"};
    constexpr int w {800}, h {800}, d {255};
    vector<int> zbuf(w * h, 0);
    PPM_Image img {w, h};

    const Vec3f light_dir {Vec3f{1, -1, 1}.normalize()}, eye {1, 1, 3},
          center {0, 0, 0};
    const Matrix ModelView {Matrix::lookat(eye, center, Vec3f{0, 1, 0})};
    const Matrix VP {Matrix::viewport(w >> 3, h >> 3, w * 3 / 4, h * 3 / 4, d)};
    //const Matrix VP {Matrix::viewport(w >> 3, h >> 3, (w >> 2) * 3,
    //        (h >> 2) * 3, d)};
    Matrix Proj {Matrix::identity()};
    Proj[3][2] = -1.0 / (eye - center).norm();
    const Matrix Z {VP * Proj * ModelView};
    //cout << ModelView << Proj << VP << Z;
    for (size_t i {0}; i < m.num_faces(); ++i) {
        const Vec3i f {m.face(i)};
        // world coordinates
        const array<Vec3f, 3> wc {m.vertex(f[0]), m.vertex(f[1]),
            m.vertex(f[2])};
        // screen coordinates
        array<Vec3i, 3> sc;
        // light intensity (kind of brightness)
        array<double, 3> br;
        for (int j {0}; j < 3; ++j) {
            sc[j] = VP * Proj * ModelView * Matrix(wc[j]);
            sc[j][1] = h - sc[j].y(); // flip upside down
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

    //test_init();
    //test_matrix();
    //test_model();
    test_camera();

    return 0;
}

