#version 460 core 

layout (std140, binding = 1) uniform CameraData {
	vec4 viewPos;
	mat4 projection;
	mat4 view;
};