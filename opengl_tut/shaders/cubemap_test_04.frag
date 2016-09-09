#version 330 core

in vec3 norm_vec;
in vec3 pos_vec;
in vec2 vert_tex;

out vec4 color;

uniform vec3 cam_pos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_reflection1;
uniform samplerCube tex_cubemap;

void main() {
    // diffuse part
    vec4 diff_color = texture(texture_diffuse1, vert_tex);

    // reflection part
    vec3 I = normalize(pos_vec - cam_pos);
    vec3 R = reflect(I, normalize(norm_vec));
    float refl_intensity = texture(texture_reflection1, vert_tex).r;
    vec4 refl_color = vec4(0, 0, 0, 0);
    // sample reflections above a threshold
    if (refl_intensity > 0.01)
        refl_color = texture(tex_cubemap, R) * refl_intensity;

    // combine diffuse and reflection parts
    color = diff_color + refl_color;
}
