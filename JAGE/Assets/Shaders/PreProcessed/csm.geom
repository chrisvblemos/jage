#version 460 core
    
layout(triangles, invocations = 4) in;
layout(triangle_strip, max_vertices = 3) out;


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

void main()
{          
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = cascades[gl_InvocationID].lightSpaceMatrix * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        
        EmitVertex();
    }
    EndPrimitive();
}
