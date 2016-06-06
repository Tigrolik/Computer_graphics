#version 330 core

in vec3 ourColor;
in vec2 ourTexCoord;

out vec4 color;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
    // linearly interpolate between both textures (second texture is only
    // slightly combined)
    color = mix(texture(ourTexture1, ourTexCoord),
            texture(ourTexture2, ourTexCoord), 0.374f);
}
