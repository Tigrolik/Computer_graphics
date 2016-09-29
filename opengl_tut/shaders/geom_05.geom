#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal_vec;
} gs_in[];

const float mag = 0.4;

void gen_line(int idx) {
    gl_Position = gl_in[idx].gl_Position;
    EmitVertex();
    gl_Position = gl_in[idx].gl_Position + vec4(gs_in[idx].normal_vec, 0) * mag;
    EmitVertex();
    EndPrimitive();
}

void main() {
    for (int i = 0; i < 3; ++i)
        gen_line(i);
}
