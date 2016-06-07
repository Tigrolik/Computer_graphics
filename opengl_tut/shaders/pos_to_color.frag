#version 330 core
// input var from the vertex shader (same name and type)
in vec3 vert_pos;
out vec4 color;
void main() {
    color = vec4(vert_pos, 1);
}
