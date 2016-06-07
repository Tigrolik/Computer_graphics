#version 330 core
// input var from the vertex shader (same name and type)
in vec4 vert_color;
out vec4 color;
void main() {
    color = vert_color;
}
