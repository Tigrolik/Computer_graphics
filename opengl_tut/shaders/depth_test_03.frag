#version 330 core

out vec4 color;

float near = 1;
float away = 100;

float linearize_depth(float depth) {
    return (2 * near * away) / (away + near - (depth * 2 - 1) * (away - near));
}

void main() {
    color = vec4(vec3(linearize_depth(gl_FragCoord.z) / away), 1);
}
