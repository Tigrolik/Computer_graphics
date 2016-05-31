#ifndef SHADER_H
#define SHADER_H

#include "Vec.h"
#include "Mat.h"
#include "Model.h"
#include "PPM_Image.h"

class IShader {
public:
    using Mat4d = Mat<4, 4, double>;

    virtual ~IShader() { }
    virtual Vec<4, double> vertex(const Model&, const Mat4d&, const Mat4d&,
            const int, const int) = 0;
    virtual bool fragment(const PPM_Image&, const Vec<3, double>&,
            PPM_Color&) = 0;
};

#endif

