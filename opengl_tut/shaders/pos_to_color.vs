#version 330 core
// the position variable has attribute position 0
layout (location = 0) in vec3 position;
// specify the color output to the fragment shader
out vec3 vert_pos;
void main() {
    gl_Position = vec4(position, 1);
    vert_pos = position;
}
