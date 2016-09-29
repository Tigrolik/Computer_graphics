#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 normal_vec;
} vs_out;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = proj * view * model * vec4(position, 1);
    mat3 normal_mat = mat3(transpose(inverse(view * model)));
    vs_out.normal_vec = normalize(vec3(proj * vec4(normal_mat * normal, 1)));
}
