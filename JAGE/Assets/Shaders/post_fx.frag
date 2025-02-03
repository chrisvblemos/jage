#version 460 core

uniform float uGamma;

layout (location = 0) out vec3 uPostFxTex2D;

layout (binding = 0) uniform sampler2D uDiffuseTex2D;

in vec2 TexCoords;

void main() {
    vec3 diffuse = texture(uDiffuseTex2D, TexCoords).rgb;

    vec3 gammaResult = diffuse * uGamma;
    uPostFxTex2D = gammaResult;
}