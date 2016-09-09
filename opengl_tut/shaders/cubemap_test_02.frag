#version 330 core

in vec3 norm_vec;
in vec3 pos_vec;

out vec4 color;

uniform vec3 cam_pos;
uniform samplerCube tex_cubemap;

void main() {
    vec3 I = normalize(pos_vec - cam_pos);
    vec3 R = reflect(I, normalize(norm_vec));
    color = texture(tex_cubemap, R);
}
