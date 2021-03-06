#version 330 core

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 pos;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant_term;
    float linear_term;
    float quadratic_term;
};
#define NR_POINT_LIGHTS 4

struct SpotLight {
    vec3 pos;
    vec3 direction;
    float cutoff;
    float outer_cutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant_term;
    float linear_term;
    float quadratic_term;
};

in vec3 normal_vec;
in vec3 frag_pos;
in vec2 vert_tex;

out vec4 color;

uniform vec3 view_pos;
uniform float shininess;

uniform DirLight dir_light;
uniform PointLight point_lights[NR_POINT_LIGHTS];
uniform SpotLight spot_light;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

// prototype for the calculation of direct light
vec3 CalcDirLight(DirLight light, vec3 norm_vec, vec3 view_dir);
vec3 CalcPointLight(PointLight light, vec3 norm_vec, vec3 frag_pos,
        vec3 view_dir);
vec3 CalcSpotLight(SpotLight light, vec3 norm_vec, vec3 frag_pos, vec3
        view_dir);

void main() {
    // set properties
    vec3 norm_vec  = normalize(normal_vec);
    vec3 view_dir  = normalize(view_pos - frag_pos);
    // phase 1: directional lighting
    vec3 res = CalcDirLight(dir_light, norm_vec, view_dir);
    // phase 2: point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i)
        res += CalcPointLight(point_lights[i], norm_vec, frag_pos, view_dir);
    // phase 3: spot light
    res += CalcSpotLight(spot_light, norm_vec, frag_pos, view_dir);
    color = vec4(res, 1);
}

// directional light calculation
vec3 CalcDirLight(DirLight light, vec3 norm_vec, vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);

    // ambient shading
    vec3 amb_vec = light.ambient * vec3(texture(texture_diffuse1, vert_tex));

    // diffuse shading
    float diff_val = max(dot(norm_vec, light_dir), 0.0);
    vec3 diff_vec  = light.diffuse * diff_val * vec3(texture(texture_diffuse1,
                vert_tex));

    // specular shading
    vec3 refl_dir  = reflect(-light_dir, norm_vec);
    float spec_val = pow(max(dot(view_dir, refl_dir), 0.0), shininess);
    vec3 spec_vec  = light.specular * spec_val *
        vec3(texture(texture_specular1, vert_tex));

    // combined
    return amb_vec + diff_vec + spec_vec;
}

// point light calculation
vec3 CalcPointLight(PointLight light, vec3 norm_vec, vec3 frag_pos,
        vec3 view_dir) {
    vec3 light_dir = normalize(light.pos - frag_pos);

    // attenuation
    float dist_val = length(light.pos - frag_pos);
    float atten_val = 1.0 / (light.constant_term + light.linear_term *
            dist_val + light.quadratic_term * (dist_val * dist_val));

    // ambient
    vec3 amb_vec = light.ambient * vec3(texture(texture_diffuse1, vert_tex)) *
        atten_val;

    // diffuse
    float diff_val = max(dot(norm_vec, light_dir), 0.0);
    vec3 diff_vec  = light.diffuse * diff_val *
        vec3(texture(texture_diffuse1, vert_tex)) * atten_val;

    // specular
    vec3 refl_dir  = reflect(-light_dir, norm_vec);
    float spec_val = pow(max(dot(view_dir, refl_dir), 0.0), shininess);
    vec3 spec_vec  = light.specular * spec_val *
        vec3(texture(texture_specular1, vert_tex)) * atten_val;

    // combined
    return amb_vec + diff_vec + spec_vec;
}

// spot light calculation
vec3 CalcSpotLight(SpotLight light, vec3 norm_vec, vec3 frag_pos,
        vec3 view_dir) {
    vec3 light_dir = normalize(light.pos - frag_pos);

    // attenuation
    float dist_val = length(light.pos - frag_pos);
    float atten_val = 1.0 / (light.constant_term + light.linear_term *
            dist_val + light.quadratic_term * (dist_val * dist_val));

    // angle and soft edges
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0, 1);

    // ambient
    vec3 amb_vec = light.ambient * vec3(texture(texture_diffuse1, vert_tex)) *
        atten_val;

    // diffuse
    float diff_val = max(dot(norm_vec, light_dir), 0.0);
    vec3 diff_vec  = light.diffuse * diff_val *
        vec3(texture(texture_diffuse1, vert_tex)) * atten_val * intensity;

    // specular
    vec3 refl_dir  = reflect(-light_dir, norm_vec);
    float spec_val = pow(max(dot(view_dir, refl_dir), 0.0), shininess);
    vec3 spec_vec  = light.specular * spec_val *
        vec3(texture(texture_specular1, vert_tex)) * atten_val * intensity;

    // combined
    return amb_vec + diff_vec + spec_vec;
}

