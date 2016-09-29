#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 vert_tex;

out VS_OUT {
    vec2 tex_coords;
} vs_out;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = proj * view * model * vec4(position, 1);
    vs_out.tex_coords = vert_tex;
}
