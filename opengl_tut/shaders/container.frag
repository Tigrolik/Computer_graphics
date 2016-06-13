#version 330 core

in vec3 vert_color;
in vec2 vert_tex;

out vec4 color;

// automatically assigned in glDrawElements()
uniform sampler2D in_texture;

void main()
{
    color = texture(in_texture, vert_tex);
}
