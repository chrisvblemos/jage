#version 460 core
#extension GL_ARB_shader_draw_parameters : enable

#include "meshes.glsl"

layout (location = 0) in vec3 aPos;
uniform mat4 uLightSpaceMatrix;

void main()
{
	uint instanceIndex = gl_InstanceID + gl_BaseInstanceARB;
	MeshInstanceData instanceData = meshInstancesDataArray[instanceIndex];

	gl_Position = uLightSpaceMatrix * instanceData.model * vec4(aPos, 1.0);
}