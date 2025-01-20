#version 460 core
#extension GL_ARB_bindless_texture : enable

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in flat int DiffuseTextureHndlrIndex;
in flat int SpecularTextureHndlrIndex;
in flat int NormalTextureHndlrIndex;

layout(std430, binding = 6) readonly buffer Tex2DSamplerDataArray {
    sampler2D tex2DSamplerDataArray[];
};

void main() {
	gPosition = FragPos;
	
	gNormal = normalize(Normal);
	if (NormalTextureHndlrIndex >= 0) {
		sampler2D tex = tex2DSamplerDataArray[NormalTextureHndlrIndex];
		gNormal = texture(tex, TexCoords).rgb;
	}

	gAlbedoSpec.rgb = vec3(0.5f, 0.0f, 0.5f);
	if (DiffuseTextureHndlrIndex >= 0) {
		sampler2D tex = tex2DSamplerDataArray[DiffuseTextureHndlrIndex];
		gAlbedoSpec.rgb = texture(tex, TexCoords).rgb;
	}

	gAlbedoSpec.a = 0.5f;
	if (SpecularTextureHndlrIndex >= 0) {
		sampler2D tex = tex2DSamplerDataArray[SpecularTextureHndlrIndex];
		gAlbedoSpec.a = texture(tex, TexCoords).r;
	}
}
