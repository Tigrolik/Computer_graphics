#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec2 vert_tex;
out vec3 normal_vec;
out vec3 frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * model * vec4(position, 1);
    // fragment's pos in world coordinates
    frag_pos = vec3(model * vec4(position, 1));
    normal_vec = normal;
}
