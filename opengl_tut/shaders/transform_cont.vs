#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 tex_coord;

out vec3 vert_color;
out vec2 vert_tex;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(position, 1);
    vert_color = color;
    // little hack to avoid drawing image upside-down
    vert_tex = vec2(tex_coord.x, 1 - tex_coord.y);
}
