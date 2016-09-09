#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out vec3 norm_vec;
out vec3 pos_vec;
out vec2 vert_tex;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main() {
    gl_Position = proj * view * model * vec4(position, 1);
    norm_vec = mat3(transpose(inverse(model))) * normal;
    pos_vec = vec3(model * vec4(position, 1));
    vert_tex = tex_coords;
}
