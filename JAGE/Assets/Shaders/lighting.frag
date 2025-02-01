#version 460 core

#include "constants.glsl"
#include "csm.glsl"
#include "camera.glsl"
#include "lighting.glsl"

in  vec2  TexCoords;
out vec4  FragColor;

layout (binding = 0) uniform samplerCubeArray   shadowCubemapArray;
layout (binding = 1) uniform sampler2DArray     shadowMapArray;
layout (binding = 2) uniform sampler2D          gPosition;
layout (binding = 3) uniform sampler2D          gNormal;
layout (binding = 4) uniform sampler2D          gAlbedoSpec;
layout (binding = 5) uniform sampler2D          gSSAO;

void main() {
	vec3  WorldFragPos   = texture(gPosition, TexCoords).rgb;        // gPosition texture
	vec3  WorldNormal    = texture(gNormal, TexCoords).rgb;          // gNormal texture
	vec3  Albedo         = texture(gAlbedoSpec, TexCoords).rgb;      // gAlbedo texture
	float Specular       = texture(gAlbedoSpec, TexCoords).a;        // gAlbedoSpec texture (stored in alpha of gAlbedo)
    float AmbientOcclusion = texture(gSSAO, TexCoords).r;
    vec3  camToFragDir   = normalize(viewPos.xyz - WorldFragPos);    // direction from camera to fragment
    vec3  lightingResult = vec3(0.0, 0.0, 0.0);                      // result of lighting calculations

    if (pointLightsCount > 0) {
	    for (int i = 0; i < pointLightsCount; ++i) {
            PointLightData pointLight   = pointLights[i];
            vec3  pointLightDir         = normalize(pointLight.position - WorldFragPos);
            float distanceToLight       = length(pointLight.position - WorldFragPos);
            float attenuation           = 1.0 / (pointLight.constant + 
                                                    pointLight.linear * distanceToLight + 
                                                    pointLight.quadratic * distanceToLight * distanceToLight);
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
                pointLight.shadowCubemapIndex, 
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

    vec3 ambientLight   = ambientLightColor * ambientLightIntensity * AmbientOcclusion;
    lightingResult      = lightingResult + ambientLight;
    vec3 result         = lightingResult * Albedo;

    FragColor = vec4(ambientLight, 1.0);
    
};

