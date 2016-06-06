#version 330 core

in vec3 ourColor;
in vec2 ourTexCoord;

out vec4 color;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
    color = mix(texture(ourTexture1, ourTexCoord), texture(ourTexture2,
                vec2(1.0f - ourTexCoord.x, ourTexCoord.y)), 0.9f);
}
