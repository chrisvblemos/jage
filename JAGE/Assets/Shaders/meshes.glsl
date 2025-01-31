#version 460 core

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
