#version 330 core
// attribute positions
layout (location = 0) in vec3 position;
// the color variable has attribute position 1
out vec4 vert_color;
uniform float x_offset;
void main() {
    gl_Position = vec4(position.x + x_offset, position.y, position.z, 1);
    vert_color = vec4(0.7, 0, 0, 1);
}

