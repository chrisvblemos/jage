#version 460 core


const vec3 CASCADE_DEBUG_COLORS[16] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(0.5, 0.0, 0.0),
    vec3(0.0, 0.5, 0.0),
    vec3(0.0, 0.0, 0.5),
    vec3(0.5, 0.5, 0.0),
    vec3(0.0, 0.5, 0.5),
    vec3(0.5, 0.0, 0.5),
    vec3(0.5, 1.0, 0.0),
    vec3(0.0, 0.5, 1.0),
    vec3(0.0, 1.0, 0.5),
    vec3(1.0, 0.5, 0.0)
);


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

layout (std140, binding = 1) uniform CameraData {
	vec4 viewPos;
	mat4 projection;
	mat4 view;
};

const float SHININESS              = 32.0;
const int   SHADOW_POISSON_SAMPLES = 4;
const float SHADOW_MIN_BIAS        = 0.005;
const float SHADOW_MAX_BIAS        = 0.05;

const vec3 POISSON_SPHERE_16[16] = vec3[](
    vec3( 0.0000,  0.0000,  1.0000),
    vec3(-0.3680,  0.3370,  0.8667),
    vec3( 0.0590, -0.6770,  0.7333),
    vec3( 0.4864,  0.6344,  0.6000),
    vec3(-0.8700, -0.1530,  0.4667),
    vec3( 0.7900, -0.5140,  0.3333),
    vec3(-0.2530,  0.9460,  0.2000),
    vec3(-0.4730, -0.8780,  0.0667),
    vec3( 0.9370,  0.3410, -0.0667),
    vec3(-0.9050,  0.3750, -0.2000),
    vec3( 0.3930, -0.8570, -0.3333),
    vec3( 0.2650,  0.8430, -0.4667),
    vec3(-0.6930, -0.4000, -0.6000),
    vec3( 0.6650, -0.1410, -0.7333),
    vec3(-0.2870,  0.4090, -0.8667),
    vec3( 0.0000,  0.0000, -1.0000)
);

const vec3 sampleOffsetDirections[20] = vec3[] (
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

struct PointLightData {
    vec3  position;
    vec3  color;
    float intensity;
    float shadowFarPlane;
    float shadowNearPlane;
    int   dataArrayIndex;
    mat4  cubemapViewMatrices[6];
};

layout(std430, binding = 4) readonly buffer PointLightDataArray {
    PointLightData pointLights[];
};

layout(std140, binding = 5) uniform SceneLightData {
    bool  hasDirectionalLight;
    vec3  directionalLightDirection;
    vec3  directionalLightColor;
    float directionalLightIntensity;
    mat4  directionalLightMatrix;
    int   pointLightsCount;
    vec3  ambientLightColor;
    float ambientLightIntensity;
};

vec3 GetLight(vec3  lightColor, 
              float lightIntensity, 
              vec3  lightDir, 
              vec3  camToFragDir, 
              vec3  normal, 
              float specular) {
    vec3 halfwayDir = normalize(lightDir + camToFragDir);
    vec3 light      = lightColor * lightIntensity;

    float diff           = max(dot(normal, lightDir), 0.0);
    vec3  diffuse        = light * diff;
    float specFactor     = pow(max(dot(normal, halfwayDir), 0.0), SHININESS); // blinn-phong
    vec3  spec           = specFactor * specular * light;

    return diffuse + spec;;
};

float GetRandomPoissonIndex(vec4 seed4) {
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
};

float LinearStep(float low, float high, float v) {
    return clamp((v - low)/(high-low), 0.0, 1.0);
}

float ChebysevShadowProb(vec2 moments, float currentDepth) {
    float p = step(currentDepth, moments.x);
    float variance  = max(moments.g - (moments.r * moments.r), 0.00001);
    float d = currentDepth - moments.r;
    float p_max = variance / (variance + d * d);
    p_max = LinearStep(0.4, 1.0, p_max);
    return min(max(p, p_max), 1.0);
};

float GetPointLightDataShadow(samplerCubeArray shadowCubemap, 
                              int   i, 
                              vec3  worldFragPos, 
                              vec3  lightPos, 
                              float farPlane, 
                              vec3  viewPos) {
    float shadow        = 0.0;
    float bias          = 0.1; 
    vec3  fragToLight   = worldFragPos - lightPos;
    float currentDepth  = length(fragToLight);
    float viewDistance  = length(viewPos - worldFragPos);
    float diskRadius    = (1.0 + (viewDistance / farPlane)) / 25.0;

    for(int j = 0; j < SHADOW_POISSON_SAMPLES; ++j)
    {
        int index = int(SHADOW_POISSON_SAMPLES * GetRandomPoissonIndex(vec4(worldFragPos * 1000.0, i))) % SHADOW_POISSON_SAMPLES;
        vec3 nudge = fragToLight + POISSON_SPHERE_16[index] * diskRadius;
        float closestDepth = texture(shadowCubemap, vec4(nudge, i)).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        
        if(currentDepth - bias > closestDepth)
            shadow += 1.0 - smoothstep(0.0, 1.0, currentDepth/farPlane);
            //shadow += 1.0;
    }

    shadow /= SHADOW_POISSON_SAMPLES;  
    return shadow;
};


float SampleVarianceShadowMap(sampler2DArray shadowMapArray, 
                              vec3  worldFragPos, 
                              vec3  lightDir, 
                              int   cascadeLayer, 
                              mat4  cascadeLightSpaceMatrix, 
                              float cascadeFarPlane) {
    
    int layer = cascadeLayer;
    
    vec4  worldFragPosLightSpace = cascadeLightSpaceMatrix * vec4(worldFragPos, 1.0);
    vec3  projCoords             = worldFragPosLightSpace.xyz / worldFragPosLightSpace.w;
          projCoords             = projCoords * 0.5 + 0.5; // to range [0,1]
    float currentDepth           = projCoords.z;
    vec2  texelSize              = 1 / vec2(textureSize(shadowMapArray, 0));

    if (currentDepth >= 1.0) return 0.0; // no shadow outside the far plane

    // poisson sampling
    float shadow = 0.0;
    for (int i = 0; i < SHADOW_POISSON_SAMPLES; i++) {
        int index = int(SHADOW_POISSON_SAMPLES * GetRandomPoissonIndex(vec4(worldFragPos * 1000.0, i))) % SHADOW_POISSON_SAMPLES;
        vec2 poissonOffset =  POISSON_SPHERE_16[index].xy * texelSize;
        vec2 moments = texture(shadowMapArray, vec3(projCoords.xy + poissonOffset, layer)).rg;

        shadow += ChebysevShadowProb(moments, currentDepth); 
    };

    shadow /= SHADOW_POISSON_SAMPLES;
    return 1.0 - shadow;
};

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

