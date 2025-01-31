#version 460 core

struct CascadeData {
    mat4 lightSpaceMatrix;
    float farPlane;
    float nearPlane;
};

uniform int cascadeCount;

layout (std140, binding = 8) uniform CascadeDataArray
{
    CascadeData cascades[16];
};

int GetCascadeLayer(vec3 worldFragPos, mat4 view) {
    vec4 worldFragPosViewSpace  = view * vec4(worldFragPos, 1.0);
    float depthValue = abs(worldFragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; i++) {
        if (depthValue < cascades[i].farPlane) {
            layer = i;
            return layer;
        }
    }

    if (layer == -1) 
        return cascadeCount - 1;
};