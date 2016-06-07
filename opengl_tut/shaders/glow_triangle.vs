#version 330 core
// attribute positions
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
// the color variable has attribute position 1
out vec3 color_val;
void main() {
    gl_Position = vec4(position, 1.0);
    color_val = color;
}
