#version 460 core

const vec3 DEFAULT_MATERIAL_DIFFUSE_COLOR = vec3(0.6, 0.8, 0.6);
const vec3 DEFAULT_MATERIAL_SPECULAR_COLOR = vec3(0.5, 0.5, 0.5);

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform bool hasDiffuseTexture = false;
uniform bool hasSpecularTexture = false;

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);

	gAlbedoSpec.rgb = DEFAULT_MATERIAL_DIFFUSE_COLOR;
	if (hasDiffuseTexture) {
		gAlbedoSpec.rgb = texture(diffuseTexture, TexCoords).rgb;
	}

	gAlbedoSpec.a = DEFAULT_MATERIAL_SPECULAR_COLOR.r;
	if (hasSpecularTexture) {
		gAlbedoSpec.a = texture(specularTexture, TexCoords).r;
	}
}
