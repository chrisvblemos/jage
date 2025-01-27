#version 460 core
layout (location = 0) out vec4 ShadowMap; // Stores depth and depth squared

void main() {
    float depth = gl_FragCoord.z;
    ShadowMap = vec4(depth, depth * depth, depth * depth * depth, depth * depth * depth * depth);
}