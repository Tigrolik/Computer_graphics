#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

// uniform color value;
out vec3 color_val;

void main() {
    gl_Position = vec4(position, 1);
    color_val = color;
}
