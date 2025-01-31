#version 460 core
#extension GL_ARB_shader_draw_parameters : enable

layout (location = 0) in vec3 aPos;

#include "meshes.glsl"

void main() {
	uint instanceIndex = gl_InstanceID + gl_BaseInstanceARB;
	MeshInstanceData instanceData = meshInstancesDataArray[instanceIndex];

	gl_Position = instanceData.model * vec4(aPos, 1.0);
}