#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 uBaseColor;
uniform float uBaseSpecular;

uniform sampler2D uDiffuseTexture;
uniform sampler2D uSpecularTexture;
uniform bool uHasDiffuseTexture = false;
uniform bool uHasSpecularTexture = false;

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);

	gAlbedoSpec.rgb = uBaseColor;
	if (uHasDiffuseTexture) {
		gAlbedoSpec.rgb = texture(uDiffuseTexture, TexCoords).rgb;
	}

	gAlbedoSpec.a = uBaseSpecular;
	if (uHasSpecularTexture) {
		gAlbedoSpec.a = texture(uSpecularTexture, TexCoords).r;
	}
}
