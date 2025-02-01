#version 460 core


layout (std140, binding = 1) uniform CameraData {
	vec4 viewPos;
	mat4 projection;
	mat4 view;
};

layout (location = 0) out float gSSAO;

layout (binding = 0) uniform sampler2D uGbuffer_Position;
layout (binding = 1) uniform sampler2D uGBuffer_Normal;
layout (binding = 2) uniform sampler2D uSSAO_TexNoise;

layout (std140, binding = 9) uniform SSAO_SettingsData {
	int uSSAO_KernelSize;
	float uSSAO_Radius;
	vec4 uSSAO_Samples[64];
	vec2 uSSAO_NoiseScale;
	float uSSAO_Bias;
};

in vec2 TexCoords;

void main() {
    vec3 fragPos = vec3(view * vec4(texture(uGbuffer_Position, TexCoords).xyz, 1.0));
    vec3 normal = normalize(mat3(view) * texture(uGBuffer_Normal, TexCoords).xyz);
    vec3 randomVec	= texture(uSSAO_TexNoise, TexCoords * uSSAO_NoiseScale).xyz; 

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 

	float occlusion = 0.0;
	for (int i = 0; i < uSSAO_KernelSize; ++i) {
		vec3 samplePos	= TBN * uSSAO_Samples[i].xyz;
		samplePos		= fragPos.xyz + samplePos * uSSAO_Radius;

		vec4 offset = vec4(samplePos, 1.0);
		offset		= projection * offset;
		offset.xyz /= offset.w;
		offset.xyz  = offset.xyz * 0.5 + 0.5;

        vec3 sampleFragPos = vec3(view * vec4(texture(uGbuffer_Position, clamp(offset.xy, 0.0, 0.95)).xyz, 1.0));
		float sampleDepth	= sampleFragPos.z;
		float rangeCheck	= smoothstep(0.0, 1.0, uSSAO_Radius / abs(fragPos.z - sampleDepth));
		occlusion		   += (sampleDepth >= samplePos.z + uSSAO_Bias ? 1.0 : 0.0) * rangeCheck;
	}

	gSSAO = 1.0 - (occlusion / uSSAO_KernelSize);
}
