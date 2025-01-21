#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D screenTexture;

void main() {
	FragColor = texture(screenTexture, TexCoords);
	// FragColor = vec4(0.5, 0, 0, 1);
}