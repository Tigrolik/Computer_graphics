#version 330 core

out vec4 color;

in vec3 normal_vec;
in vec3 frag_pos;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;

void main()
{
    // ambient part
    float amb_strength = 0.1;
    vec3 amb_vec = amb_strength * light_color;

    // diffuse part
    vec3 norm_vec = normalize(normal_vec);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff_val = max(dot(norm_vec, light_dir), 0.0);
    vec3 diff_vec = diff_val * light_color;

    // combined
    vec3 res = (amb_vec + diff_vec) * object_color;

    color = vec4(res, 1);
}
