#version 330 core

in vec3 lighting_color;

out vec4 color;

uniform vec3 object_color;

void main() {
    color = vec4(lighting_color * object_color, 1);
}
