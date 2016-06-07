#version 330 core

in vec3 color_val;
out vec4 color;

void main() {
    color = vec4(color_val, 1);
}
