#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec2 vert_tex;

void main()
{
    gl_Position = vec4(position.x, position.y, 0, 1);
    vert_tex = tex_coord;
}
