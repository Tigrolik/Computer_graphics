#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 tex_coords;
} gs_in[];

out vec2 vert_tex;

uniform float time_value;

vec3 get_normal() {
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 pos_vec, vec3 norm_vec) {
    float magnitude = 2.0;
    vec3 dir_vec = norm_vec * ((sin(time_value) + 1.0) / 2.0) * magnitude;
    return pos_vec + vec4(dir_vec, 0);
}

void main() {
    vec3 normal_vec = get_normal();

    for (int i = 0; i < 3; ++i) {
        gl_Position = explode(gl_in[i].gl_Position, normal_vec);
        vert_tex = gs_in[i].tex_coords;
        EmitVertex();
    }

    EndPrimitive();
}
