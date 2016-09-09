#version 330 core

in vec3 vert_tex;

out vec4 color;

// cubemap texture sampler
uniform samplerCube tex_cubemap;

void main() {
    color = texture(tex_cubemap, vert_tex);
}
