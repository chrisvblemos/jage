#version 460 core

in vec4 FragPos;

#include "lighting.glsl"

uniform int uPointLightIndex;

void main() {
vec3 pos = pointLights[uPointLightIndex].position;
float farPlane = pointLights[uPointLightIndex].shadowFarPlane;

float lightDist = length(FragPos.xyz - pos);
lightDist = lightDist / farPlane;
gl_FragDepth = lightDist;
}