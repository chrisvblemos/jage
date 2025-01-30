#version 460 core
#extension GL_ARB_shader_draw_parameters : enable

layout (location = 0) in vec3 aPos;

struct MeshInstanceData {
	mat4 model;
	mat4 inverseModel;
};

layout(std430, binding = 2) readonly buffer MeshInstanceDataArray {
    MeshInstanceData meshInstancesDataArray[];
};

void main() {
	uint instanceIndex = gl_InstanceID + gl_BaseInstanceARB;
	MeshInstanceData instanceData = meshInstancesDataArray[instanceIndex];

	gl_Position = instanceData.model * vec4(aPos, 1.0);
}