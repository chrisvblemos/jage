#version 460 core
#extension GL_ARB_shader_draw_parameters : enable


layout (std140, binding = 1) uniform CameraData {
	vec4 viewPos;
	mat4 projection;
	mat4 view;
};

struct DrawMeshCommandData {
	uint count;
	uint instanceCount;
	uint firstIndex;
	uint baseVertex;
	uint baseInstance;

	int diffTexHndlrIndex;
	int specTexHndlrIndex;
	int normTexHndlrIndex;
};

struct MeshInstanceData {
	mat4 model;
	mat4 inverseModel;
};

layout(std430, binding = 2) readonly buffer MeshInstanceDataArray {
    MeshInstanceData meshInstancesDataArray[];
};

layout(std430, binding = 7) readonly buffer DrawCmdsDataArray {
    DrawMeshCommandData drawCmdsDataArray[];
};

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out flat int DiffuseTextureHndlrIndex;
out flat int SpecularTextureHndlrIndex;
out flat int NormalTextureHndlrIndex;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

void main()
{
	 uint instanceIndex = gl_InstanceID + gl_BaseInstanceARB;
	 MeshInstanceData instanceData = meshInstancesDataArray[instanceIndex];

	 gl_Position = projection * view * instanceData.model * vec4(aPos, 1.0);

	 TexCoords = aTexCoords;
	 FragPos = vec3(instanceData.model * vec4(aPos, 1.0));
	 Normal = mat3(instanceData.inverseModel) * aNormal;

	 int diffTexHndlrIndex = drawCmdsDataArray[gl_DrawID].diffTexHndlrIndex;
	 int specTexHndlrIndex = drawCmdsDataArray[gl_DrawID].specTexHndlrIndex;
	 int normTexHndlrIndex = drawCmdsDataArray[gl_DrawID].normTexHndlrIndex;

	 DiffuseTextureHndlrIndex = diffTexHndlrIndex;
	 SpecularTextureHndlrIndex = specTexHndlrIndex;
	 NormalTextureHndlrIndex = normTexHndlrIndex;
}
