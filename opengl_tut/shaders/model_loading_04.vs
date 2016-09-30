#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in mat4 instance_mat;

out vec2 vert_tex;

uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * instance_mat * vec4(position, 1.0f);
    vert_tex = tex_coord;
}
