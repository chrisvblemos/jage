#version 460 core

#include "constants.glsl"
#include "samplers.glsl"
#include "camera.glsl"

const vec3 DEFAULT_BASE_ALBEDO = vec3(1.0, 1.0, 1.0);
const float DEFAULT_BASE_SPEC = 0.2;
const float DEFAULT_BASE_METAL = 0.0;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out float gSpecular;
layout (location = 4) out float gMetallic;
layout (location = 5) out vec4 gViewPosition;
layout (location = 6) out vec3 gViewNormal;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

in flat int NormalTextureHndlrIndex;
in flat int DiffuseTextureHndlrIndex;
in flat int SpecularTextureHndlrIndex;
in flat int MetallicTextureHndlrIndex;

void main() {
	gPosition = FragPos;
	gViewPosition = view * vec4(FragPos, 1.0);
	
	gNormal = normalize(Normal);
	gViewNormal = normalize(mat3(view) * Normal);
	if (NormalTextureHndlrIndex >= 0) {
		vec3 normalTexture = GetTexture(NormalTextureHndlrIndex, TexCoords).rgb;
		gNormal = normalize(normalTexture);
		gViewNormal = normalize(mat3(view) * normalTexture);
	}

	gAlbedo = DEFAULT_BASE_ALBEDO;
	if (DiffuseTextureHndlrIndex >= 0) {
		gAlbedo = GetTexture(DiffuseTextureHndlrIndex, TexCoords).rgb;
	}

	gSpecular = DEFAULT_BASE_SPEC;
	if (SpecularTextureHndlrIndex >= 0) {
		gSpecular = GetTexture(SpecularTextureHndlrIndex, TexCoords).r;
	}

	gMetallic = DEFAULT_BASE_METAL;
	if (MetallicTextureHndlrIndex >= 0) {
		gMetallic = GetTexture(MetallicTextureHndlrIndex, TexCoords).r;
	}
}