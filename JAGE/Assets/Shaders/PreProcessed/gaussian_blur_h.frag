#version 460 core


vec3 ApplyHorizontalBlur(sampler2D texsampler, vec2 texelSize, vec2 texCoords, float blurAmount);
vec3 ApplyVerticalBlur(sampler2D texsampler, vec2 texelSize, vec2 texCoords, float blurAmount);

vec3 ApplyHorizontalBlur(sampler2D texSampler, vec2 texelSize, vec2 texCoords, float blurAmount) {
    vec3 result = texture(texSampler, texCoords).rgb * 0.2270270270;

    // Gaussian kernel weights
    result += texture(texSampler, texCoords + texelSize.x * 1.0 * blurAmount).rgb * 0.1945945946;
    result += texture(texSampler, texCoords - texelSize.x * 1.0 * blurAmount).rgb * 0.1945945946;
    result += texture(texSampler, texCoords + texelSize.x * 2.0 * blurAmount).rgb * 0.1216216216;
    result += texture(texSampler, texCoords - texelSize.x * 2.0 * blurAmount).rgb * 0.1216216216;
    result += texture(texSampler, texCoords + texelSize.x * 3.0 * blurAmount).rgb * 0.0540540541;
    result += texture(texSampler, texCoords - texelSize.x * 3.0 * blurAmount).rgb * 0.0540540541;
    result += texture(texSampler, texCoords + texelSize.x * 4.0 * blurAmount).rgb * 0.0162162162;
    result += texture(texSampler, texCoords - texelSize.x * 4.0 * blurAmount).rgb * 0.0162162162;

    return result;
};

vec3 ApplyVerticalBlur(sampler2D texsampler, vec2 texelSize, vec2 texCoords, float blurAmount) {
    vec3 result = texture(texsampler, texCoords).rgb * 0.2270270270;

    result += texture(texsampler, texCoords + texelSize.y * 1.0 * blurAmount).rgb * 0.1945945946;
    result += texture(texsampler, texCoords - texelSize.y * 1.0 * blurAmount).rgb * 0.1945945946;
    result += texture(texsampler, texCoords + texelSize.y * 2.0 * blurAmount).rgb * 0.1216216216;
    result += texture(texsampler, texCoords - texelSize.y * 2.0 * blurAmount).rgb * 0.1216216216;
    result += texture(texsampler, texCoords + texelSize.y * 3.0 * blurAmount).rgb * 0.0540540541;
    result += texture(texsampler, texCoords - texelSize.y * 3.0 * blurAmount).rgb * 0.0540540541;
    result += texture(texsampler, texCoords + texelSize.y * 4.0 * blurAmount).rgb * 0.0162162162;
    result += texture(texsampler, texCoords - texelSize.y * 4.0 * blurAmount).rgb * 0.0162162162;

    return result;
};

out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 0) uniform sampler2D shadowMap;

void main() {
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    vec3 result = ApplyHorizontalBlur(shadowMap, texelSize, TexCoords, 5.0);
    FragColor = vec4(result, 1.0);
};

