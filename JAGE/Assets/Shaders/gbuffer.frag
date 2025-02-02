#version 460 core

#include "constants.glsl"
#include "samplers.glsl"
#include "camera.glsl"

const vec3 DEFAULT_BASE_COLOR = vec3(1.0, 1.0, 1.0);
const float DEFAULT_BASE_SPEC = 0.2;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gViewPosition;
layout (location = 4) out vec3 gViewNormal;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in flat int DiffuseTextureHndlrIndex;
in flat int SpecularTextureHndlrIndex;
in flat int NormalTextureHndlrIndex;

void main() {
	gPosition = FragPos;
	gViewPosition = view * vec4(FragPos, 1.0);
	gNormal = normalize(Normal);
	gAlbedoSpec = vec4(0.5, 0, 0, 0.2);
	
	gNormal = normalize(Normal);
	gViewNormal = normalize(mat3(view) * Normal);
	if (NormalTextureHndlrIndex >= 0) {
		vec3 normalTexture = GetTexture(NormalTextureHndlrIndex, TexCoords).rgb;
		gNormal = normalize(normalTexture);
		gViewNormal = normalize(mat3(view) * normalTexture);
	}

	gAlbedoSpec.rgb = DEFAULT_BASE_COLOR;
	if (DiffuseTextureHndlrIndex >= 0) {
		gAlbedoSpec.rgb = GetTexture(DiffuseTextureHndlrIndex, TexCoords).rgb;
	}

	gAlbedoSpec.a = DEFAULT_BASE_SPEC;
	if (SpecularTextureHndlrIndex >= 0) {
		gAlbedoSpec.a = GetTexture(SpecularTextureHndlrIndex, TexCoords).a;
	}
}