#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 tex_coord;

out vec2 vert_tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * model * vec4(position, 1);
    vert_tex = vec2(tex_coord.x, 1 - tex_coord.y);
}
