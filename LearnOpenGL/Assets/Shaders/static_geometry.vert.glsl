#version 460 core

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout(std140, binding = 1) uniform CameraBlock {
    vec4 uCamViewPos;
    mat4 uCamViewMat;
    mat4 uCamProjMat;
};

uniform mat4 model;
uniform mat4 invModelT;

void main()
{
	gl_Position = uCamProjMat * uCamViewMat * model * vec4(aPos, 1.0);

	TexCoords = aTexCoords;
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(invModelT) * aNormal;
}