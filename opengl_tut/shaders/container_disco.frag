#version 330 core

in vec3 vert_color;
in vec2 vert_tex;

out vec4 color;

uniform sampler2D in_texture;

void main()
{
    color = texture(in_texture, vert_tex) * vec4(vert_color, 1);
}
