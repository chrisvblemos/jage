#version 460 core
    
layout(triangles, invocations = 4) in;
layout(triangle_strip, max_vertices = 3) out;

#include "csm.glsl"

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