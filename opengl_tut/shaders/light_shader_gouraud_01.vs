#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 lighting_color;

uniform vec3 light_pos;
uniform vec3 view_pos;
uniform vec3 light_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * model * vec4(position, 1);

    // Gouraud part
    vec3 gouraud_pos = vec3(model * vec4(position, 1));
    vec3 gouraud_norm = mat3(transpose(inverse(model))) * normal;

    // ambient part
    float amb_strength = 0.1;
    vec3 amb_vec = amb_strength * light_color;

    // diffuse part
    vec3 norm_vec = normalize(gouraud_norm);
    vec3 light_dir = normalize(light_pos - gouraud_pos);
    float diff_val = max(dot(norm_vec, light_dir), 0.0);
    vec3 diff_vec = diff_val * light_color;

    // specular part
    float spec_strength = 0.5;
    vec3 view_dir = normalize(view_pos - gouraud_pos);
    vec3 refl_dir = reflect(-light_dir, norm_vec);
    float spec_val = pow(max(dot(view_dir, refl_dir), 0.0), 32);
    vec3 spec_vec = spec_strength * spec_val * light_color;

    lighting_color = amb_vec + diff_vec + spec_vec;
}
