#version 330 core

in vec2 vert_tex;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main() {
    color = texture(texture_diffuse1, vert_tex);
    // convert rgb color to grayscale
    float avg = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = vec4(avg, avg, avg, 1);
}
