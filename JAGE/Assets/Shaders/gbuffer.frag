#version 460 core
#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_gpu_shader_int64 : enable

const vec4 MISSING_TEXTURE_DEFAULT = vec4(0.5, 0, 0.5, 1);	// pinkish color
const vec4 INVALID_TEXTURE_DEFAULT = vec4(1, 0, 0, 1);		// red color
const vec3 DEFAULT_BASE_COLOR = vec3(1.0, 1.0, 1.0);		// white color
const float DEFAULT_BASE_SPEC = 0.2;

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
    uint64_t tex2DSamplerDataArray[];
};

vec4 GetTexture(int handleIndex, vec2 uv);

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);
	gAlbedoSpec = vec4(0.5, 0, 0, 0.2);
	
	gNormal = normalize(Normal);
	if (NormalTextureHndlrIndex >= 0) {
		vec3 normalTexture = GetTexture(NormalTextureHndlrIndex, TexCoords).rgb;
		gNormal = normalize(normalTexture);
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

vec4 GetTexture(int handleIndex, vec2 uv) {
	if (handleIndex < 0) return MISSING_TEXTURE_DEFAULT;

	uint64_t handle = tex2DSamplerDataArray[handleIndex];
	if (handle == 0) return INVALID_TEXTURE_DEFAULT;

	sampler2D sampler = sampler2D(handle);
	return texture(sampler, uv);
}
