#version 460 core

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// instance data
uniform mat4 model;
uniform mat4 inverseModel;

layout (std140, binding = 0) uniform CameraData {
	vec4 viewPos;
	mat4 projection;
	mat4 view;
};

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	TexCoords = aTexCoords;
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(inverseModel) * aNormal;
}