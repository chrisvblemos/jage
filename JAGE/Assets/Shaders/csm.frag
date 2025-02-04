#version 460 core
layout (location = 0) out vec2 ShadowMap; // Stores depth and depth squared

void main() {
    float depth = gl_FragCoord.z;
    float dx = dFdx(depth);
    float dy = dFdy(depth);
    ShadowMap = vec2(depth, depth * depth + 0.25 * (dx * dx + dy * dy));
}