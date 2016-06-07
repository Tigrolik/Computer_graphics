#version 330 core
out vec4 color;
uniform vec4 color_val; // set this in OpenGL
void main() {
    color = color_val;
}
