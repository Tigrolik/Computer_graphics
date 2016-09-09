#version 330 core

in vec2 vert_tex;

out vec4 color;

uniform sampler2D tex_map;

const float offset = 1.0 / 300;

void main() {
    // offsets
    vec2 offsets[9] = vec2[]
        (
         vec2(-offset, offset),  // top-left
         vec2(0,       offset),  // top-center
         vec2(offset,  offset),  // top-right
         vec2(-offset, 0),       // center-left
         vec2(0,       0),       // center-center
         vec2(offset,  0),       // center-right
         vec2(-offset, -offset), // bottom-left
         vec2(0,       -offset), // bottom-center
         vec2(offset,  -offset)  // bottom-right
        );

    // define the sharpen kernel
    float kernel[9] = float[]
        (
         -1, -1, -1,
         -1,  9, -1,
         -1, -1, -1
        );

    // implementing convolution
    vec3 sample_tex[9];
    vec3 col = vec3(0);
    for (int i = 0; i < 9; ++i) {
        sample_tex[i] = vec3(texture(tex_map, vert_tex.st + offsets[i]));
        col += sample_tex[i] * kernel[i];
    }

    color = vec4(col, 1);
}
