#version 330 core

in vec2 vert_tex;

out vec4 color;

uniform sampler2D in_tex1;
uniform sampler2D in_tex2;

void main()
{
    color = mix(texture(in_tex1, vert_tex), texture(in_tex2, vert_tex), 0.375);
}
