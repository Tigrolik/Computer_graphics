#version 330 core

out vec4 color;

uniform vec3 lamp_color;

void main()
{
    color = vec4(lamp_color, 1);
}
