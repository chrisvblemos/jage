#version 460 core

in vec4 FragPos;

uniform vec3 uLightPos;
uniform float uLightFarPlane;

void main() {
	float lightDist = length(FragPos.xyz - uLightPos);
	lightDist = lightDist / uLightFarPlane;
	gl_FragDepth = lightDist;
}