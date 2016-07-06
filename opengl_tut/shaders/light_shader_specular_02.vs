#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 normal_vec;
out vec3 frag_pos;
out vec3 light_pos_vs;

uniform vec3 light_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * model * vec4(position, 1);
    frag_pos = vec3(view * model * vec4(position, 1));
    normal_vec = mat3(transpose(inverse(view * model))) * normal;
    light_pos_vs = vec3(view * vec4(light_pos, 1));
}
