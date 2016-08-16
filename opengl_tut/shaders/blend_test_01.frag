#version 330 core

in vec2 vert_tex;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main() {
    vec4 color_tex = texture(texture_diffuse1, vert_tex);
    if (color_tex.a < 0.1)
        discard;
    color = color_tex;
}
