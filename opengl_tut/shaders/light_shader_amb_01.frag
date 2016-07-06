#version 330 core

out vec4 color;

uniform vec3 object_color;
uniform vec3 light_color;

void main()
{
    const float amb_strength = 0.1;
    color = vec4((amb_strength * light_color) * object_color, 1);
}
