#version 460 core
#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_gpu_shader_int64 : enable

const vec4 MISSING_TEXTURE_DEFAULT = vec4(0.5, 0, 0.5, 1);	// pinkish color
const vec4 INVALID_TEXTURE_DEFAULT = vec4(1, 0, 0, 1);		// red color

layout(std430, binding = 6) readonly buffer Tex2DSamplerDataArray {
    uint64_t tex2DSamplerDataArray[];
};

vec4 GetTexture(int handleIndex, vec2 uv);

vec4 GetTexture(int handleIndex, vec2 uv) {
	if (handleIndex < 0) return MISSING_TEXTURE_DEFAULT;

	uint64_t handle = tex2DSamplerDataArray[handleIndex];
	if (handle == 0) return INVALID_TEXTURE_DEFAULT;

	sampler2D sampler = sampler2D(handle);
	return texture(sampler, uv);
}
