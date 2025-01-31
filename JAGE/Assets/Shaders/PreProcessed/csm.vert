#version 460 core
#extension GL_ARB_shader_draw_parameters : enable


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

layout (location = 0) in vec3 aPos;

void main() {
	uint instanceIndex = gl_InstanceID + gl_BaseInstanceARB;
	MeshInstanceData instanceData = meshInstancesDataArray[instanceIndex];

	gl_Position = instanceData.model * vec4(aPos, 1.0);
}
