#version 460 core

#include "constants.glsl"
#include "csm.glsl"
#include "camera.glsl"
#include "lighting.glsl"

in  vec2  TexCoords;

layout (location = 0) out vec3 outDiffuse;

layout (binding = 0) uniform samplerCubeArray   shadowCubemapArray;
layout (binding = 1) uniform sampler2DArray     shadowMapArray;
layout (binding = 2) uniform sampler2D          gPosition;
layout (binding = 3) uniform sampler2D          gNormal;
layout (binding = 4) uniform sampler2D          gAlbedo;
layout (binding = 5) uniform sampler2D          gSpecular;
layout (binding = 6) uniform sampler2D          gMetallic;
layout (binding = 7) uniform sampler2D          gSSAO;

void main() {
	vec3  WorldFragPos     = texture(gPosition, TexCoords).rgb;
	vec3  WorldNormal      = texture(gNormal, TexCoords).rgb;
	vec3  Albedo           = texture(gAlbedo, TexCoords).rgb;
	float Specular         = texture(gSpecular, TexCoords).a;
    float Metallic         = texture(gMetallic, TexCoords).r;
    float AmbientOcclusion = texture(gSSAO, TexCoords).r;
    vec3  camToFragDir     = normalize(viewPos.xyz - WorldFragPos);
    vec3  lightingResult   = vec3(0.0, 0.0, 0.0);

    if (pointLightsCount > 0) {
	    for (int i = 0; i < pointLightsCount; ++i) {
            PointLightData pointLight   = pointLights[i];
            vec3  pointLightDir         = normalize(pointLight.position - WorldFragPos);
            float distanceToLight       = length(pointLight.position - WorldFragPos);
            float attenuation           = 1.0 / (distanceToLight * distanceToLight);
            float attenuatedIntensity   = pointLight.intensity * attenuation;

            vec3 pointLightResult = GetLight(
                pointLight.color, 
                attenuatedIntensity, 
                pointLightDir, 
                camToFragDir, 
                WorldNormal, 
                Specular
            );

            float shadow = GetPointLightDataShadow(
                shadowCubemapArray,
                pointLight.dataArrayIndex, 
                WorldFragPos, 
                pointLight.position, 
                pointLight.shadowFarPlane,
                viewPos.xyz
            );
            
		    lightingResult += (1.0 - shadow) * pointLightResult;
	    };
    };

    if (hasDirectionalLight) {
        vec3 lightDir = normalize(directionalLightDirection);
        
        vec3 directionalLighting = GetLight(
            directionalLightColor, 
            directionalLightIntensity, 
            -lightDir, 
            camToFragDir, 
            WorldNormal, 
            Specular
        );

        int cascadeLayer = GetCascadeLayer(WorldFragPos, view);
        mat4 cascadeLightMatrix = cascades[cascadeLayer].lightSpaceMatrix;
        float cascadeFarPlane = cascades[cascadeLayer].farPlane;
        float shadow = SampleVarianceShadowMap(shadowMapArray, WorldFragPos, lightDir, cascadeLayer, cascadeLightMatrix, cascadeFarPlane);
        lightingResult += (1.0 - shadow) * directionalLighting;
    };

    vec3 ambientLight   = ambientLightColor * ambientLightIntensity;
    lightingResult      = (lightingResult + ambientLight) * AmbientOcclusion;
    outDiffuse         = lightingResult * Albedo;
};

