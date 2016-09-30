#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 offset;

out vec3 frag_color;

void main() {
    vec2 pos = position * (max(0.1, gl_InstanceID / 100.0));
    gl_Position = vec4(pos + offset, 0, 1);
    frag_color = color;
}
