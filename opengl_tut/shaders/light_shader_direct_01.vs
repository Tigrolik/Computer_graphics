#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

out vec2 vert_tex;
out vec3 normal_vec;
out vec3 frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * model * vec4(position, 1);
    frag_pos = vec3(model * vec4(position, 1));
    normal_vec = mat3(transpose(inverse(model))) * normal;
    vert_tex = tex_coord;
}
