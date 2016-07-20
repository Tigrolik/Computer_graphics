#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 normal_vec;
in vec3 frag_pos;

out vec4 color;

uniform vec3 view_pos;
uniform Material mater;
uniform Light light;

void main()
{
    // ambient part
    vec3 amb_vec = light.ambient * mater.ambient;

    // diffuse part
    vec3 norm_vec  = normalize(normal_vec);
    vec3 light_dir = normalize(light.pos - frag_pos);
    float diff_val = max(dot(norm_vec, light_dir), 0.0);
    vec3 diff_vec  = light.diffuse * (diff_val * mater.diffuse);

    // specular part
    vec3 view_dir  = normalize(view_pos - frag_pos);
    vec3 refl_dir  = reflect(-light_dir, norm_vec);
    float spec_val = pow(max(dot(view_dir, refl_dir), 0.0), mater.shininess);
    vec3 spec_vec  = light.specular * (spec_val * mater.specular);

    // combined
    vec3 res = amb_vec + diff_vec + spec_vec;
    color = vec4(res, 1);
}
