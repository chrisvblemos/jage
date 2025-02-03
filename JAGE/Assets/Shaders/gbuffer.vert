#version 460 core
#extension GL_ARB_shader_draw_parameters : enable

#include "camera.glsl"
#include "meshes.glsl"

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out flat int DiffuseTextureHndlrIndex;
out flat int SpecularTextureHndlrIndex;
out flat int NormalTextureHndlrIndex;
out flat int MetallicTextureHndlrIndex;

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
	 MetallicTextureHndlrIndex = -1;
}