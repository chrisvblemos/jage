#version 460 core

uniform float uGamma;

layout (location = 0) out vec3 uPostFxTex2D;

layout (binding = 0) uniform sampler2D uDiffuseTex2D;

in vec2 TexCoords;

void main() {
    vec3 diffuse = texture(uDiffuseTex2D, TexCoords).rgb;
    diffuse = diffuse / (diffuse + vec3(1.0));  // tone mapping
    diffuse = vec3(1.0) - exp(-diffuse * 1.0);  // exposure
    diffuse = pow(diffuse, vec3(1.0/uGamma));   // gamma
    uPostFxTex2D = diffuse;
}