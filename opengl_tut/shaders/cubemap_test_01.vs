#version 330 core

layout (location = 0) in vec3 position;

out vec3 vert_tex;

uniform mat4 view;
uniform mat4 proj;

void main() {
    vec4 pos = proj * view * vec4(position, 1);
    gl_Position = pos.xyww;
    vert_tex = position;
}
