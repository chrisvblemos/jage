#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

#include "lighting.glsl"

uniform int uPointLightIndex;

out vec4 FragPos;

void main() {
	for(int face=0; face < 6; ++face) {
		gl_Layer = 6 * uPointLightIndex + face;

		for(int i = 0; i < 3; ++i) {
			FragPos = gl_in[i].gl_Position;
			gl_Position = pointLights[uPointLightIndex].cubemapViewMatrices[face] * FragPos;
			EmitVertex();
		}

		EndPrimitive();
	}
}