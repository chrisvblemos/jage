#version 460 core

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

out vec4 FragColor;

void main()
{
    vec3 BaseColor = vec3(texture(gAlbedoSpec, TexCoords).rgb);
    FragColor = vec4(BaseColor, 1.0);
}


