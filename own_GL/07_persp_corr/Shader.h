#ifndef SHADER_H
#define SHADER_H

#include "Vec.h"
#include "Mat.h"
#include "Model.h"
#include "PPM_Image.h"

// interface class
class IShader {
public:
    using Mat4d = Mat<4, 4, double>;

    virtual ~IShader() { }

    virtual Vec<4, double> vertex(const Model&, const Mat4d&, const Mat4d&,
            const Mat4d&, const Vec<3, double>&, const int, const int) = 0;
    virtual bool fragment(const PPM_Image&, const Vec<3, double>&,
            PPM_Color&) = 0;
};

// Gouraud shader lcass
class Gouraud_shader: public IShader {
public:
    using Vec3d = Vec<3, double>;
    using Vec4d = Vec<4, double>;

    Vec4d vertex(const Model &m, const Mat4d &Viewport, const Mat4d &Proj,
            const Mat4d &ModelView, const Vec3d &L_dir, const int iface,
            const int ivert) {
        // get vertex from the model file
        Vec4d gl_vert = resize<4>(m.vertex(iface, ivert));
        // transform the vertex to screen coordinates
        gl_vert = Viewport * Proj * ModelView * gl_vert;
        // get diffuse lighting intensity
        var_intensity[ivert] = std::max(0.0, m.normal(iface, ivert) * L_dir);
        return gl_vert;
    }

    bool fragment(const PPM_Image&, const Vec3d &bar, PPM_Color &C) {
        // interpolate intensity for the current pixel
        double intensity = var_intensity * bar;

        // playing with limited number of intensity values
        //if (intensity > 0.85)
        //    intensity = 1;
        //else if (intensity > 0.6)
        //    intensity = 0.8;
        //else if (intensity > 0.45)
        //    intensity = 0.6;
        //else if (intensity > 0.3)
        //    intensity = 0.45;
        //else if (intensity > 0.15)
        //    intensity = 0.3;
        //else
        //    intensity = 0;

        C = PPM_Color{255, 255, 255} * intensity;
        //C = PPM_Color{255, 155, 0} * intensity;
        return false; // we don't discard this pixel
    }

private:
    Vec3d var_intensity {};
};

// texture shader
class Tex_shader: public IShader {
public:
    using Vec3d = Vec<3, double>;
    using Vec4d = Vec<4, double>;
    Vec4d vertex(const Model &m, const Mat4d &Viewport, const Mat4d &Proj,
            const Mat4d &ModelView, const Vec3d &L_dir, const int iface,
            const int ivert) {
        var_uv.fill_col(ivert, resize<2>(m.texvertex(iface, ivert)));
        var_intensity[ivert] = std::max(0.0, m.normal(iface, ivert) * L_dir);
        Vec4d gl_vert = resize<4>(m.vertex(iface, ivert));
        gl_vert = Viewport * Proj * ModelView * gl_vert;
        return gl_vert;
    }

    bool fragment(const PPM_Image &tex, const Vec3d &bar, PPM_Color &C) {
        double intensity = var_intensity * bar;
        auto uv = var_uv * bar;
        C = tex.color(uv.x() * tex.width(), tex.height() * (1 - uv.y())) *
            intensity;
        return false;
    }

private:
    Vec3d var_intensity {};
    // triangle uv coordinates: written by the vertex shader, read by the
    // fragment shader
    Mat<2, 3, double> var_uv {};
};

void triangle_shader(const Mat<3, 4, double>&, IShader&, PPM_Image&,
        const PPM_Image&, std::vector<int>&);

#endif

