#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 0) uniform sampler2D shadowMap;
uniform float blurScale;

void main() {
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    vec3 result = texture(shadowMap, TexCoords).rgb * 0.2270270270;

    // Gaussian kernel weights
    result += texture(shadowMap, TexCoords + texelSize.x * 1.0 * blurScale).rgb * 0.1945945946;
    result += texture(shadowMap, TexCoords - texelSize.x * 1.0 * blurScale).rgb * 0.1945945946;
    result += texture(shadowMap, TexCoords + texelSize.x * 2.0 * blurScale).rgb * 0.1216216216;
    result += texture(shadowMap, TexCoords - texelSize.x * 2.0 * blurScale).rgb * 0.1216216216;
    result += texture(shadowMap, TexCoords + texelSize.x * 3.0 * blurScale).rgb * 0.0540540541;
    result += texture(shadowMap, TexCoords - texelSize.x * 3.0 * blurScale).rgb * 0.0540540541;
    result += texture(shadowMap, TexCoords + texelSize.x * 4.0 * blurScale).rgb * 0.0162162162;
    result += texture(shadowMap, TexCoords - texelSize.x * 4.0 * blurScale).rgb * 0.0162162162;

    FragColor = vec4(result, 1.0);
}