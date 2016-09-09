#version 330 core

in vec2 vert_tex;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main() {
    color = vec4(vec3(1 - texture(texture_diffuse1, vert_tex)), 1);
}
