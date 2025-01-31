#version 460 core

#include "gaussian_blur.glsl"

out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 0) uniform sampler2D blurredShadowMap;

void main() {
    vec2 texelSize = 1.0 / textureSize(blurredShadowMap, 0);
    vec3 result = ApplyVerticalBlur(blurredShadowMap, texelSize, TexCoords, 5.0);

    FragColor = vec4(result, 1.0);
};

