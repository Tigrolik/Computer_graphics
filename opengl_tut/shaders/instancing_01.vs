#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

out vec3 frag_color;

uniform vec2 offsets_arr[100];

void main() {
    vec2 offset_vec = offsets_arr[gl_InstanceID];
    gl_Position = vec4(position + offset_vec, 0, 1);
    frag_color = color;
}
