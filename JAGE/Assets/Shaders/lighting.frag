#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout (binding = 0) uniform samplerCubeArray shadowCubemapArray;
layout (binding = 1) uniform sampler2D directionalLightShadowMap;
layout (binding = 2) uniform sampler2D gPosition;
layout (binding = 3) uniform sampler2D gNormal;
layout (binding = 4) uniform sampler2D gAlbedoSpec;

const float SHININESS = 32.0;
const int POISSON_SAMPLES = 16;

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

const vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

struct PointLightData {
    vec3 position;
    float radius;
    vec3 color;
    float intensity;
    float shadowFarPlane;
    int shadowCubemapIndex;
    float dataArrayIndex;
    float constant;
    float linear;
    float quadratic;
};

layout(std430, binding = 4) readonly buffer PointLightDataArray {
    PointLightData pointLights[];
};

layout(std140, binding = 5) uniform SceneLightData {
    bool hasDirectionalLight;
    vec3 directionalLightDirection;
    vec3 directionalLightColor;
    float directionalLightIntensity;
    mat4 directionalLightMatrix;
    int pointLightsCount;
    vec3 ambientLightColor;
    float ambientLightIntensity;
};

layout (std140, binding = 1) uniform CameraData {
	vec4 viewPos;
};

// internal functions
vec3 GetLight(vec3 lightColor, float lightIntensity, vec3 lightDir, vec3 viewDir, vec3 normal, float specular);
float GetDirectLightShadow(vec3 fragPos, float bias);
float GetPointLightDataShadow(int i, vec3 fragPos, vec3 lightPos, float farPlane);
float GetRandomPoissonIndex(vec4 seed4);
float ChebysevUpperBound(vec4 moments, float currentDepth);
float GetChebysevShadow(vec3 fragPos);

void main() {
	vec3 FragPos = texture(gPosition, TexCoords).rgb;       // gPosition texture
	vec3 Normal = texture(gNormal, TexCoords).rgb;          // gNormal texture
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;      // gAlbedo texture
	float Specular = texture(gAlbedoSpec, TexCoords).a;     // gAlbedoSpec texture (stored in alpha of gAlbedo)
    
    vec3 viewDir = normalize(viewPos.xyz - FragPos);            // direction from camera to fragment

    vec3 lightingResult = vec3(0.0, 0.0, 0.0);              // result of lighting calculations

    // point lights
    if (pointLightsCount > 0) {
	    for (int i = 0; i < pointLightsCount; ++i) {
            PointLightData pointLight = pointLights[i];
            vec3 pointLightDir = normalize(pointLight.position - FragPos);
            float distanceToLight = length(pointLight.position - FragPos);

            float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distanceToLight + pointLight.quadratic * distanceToLight * distanceToLight);
            float attenuatedIntensity = pointLight.intensity * attenuation;

            vec3 pointLightResult = GetLight(
                pointLight.color, 
                attenuatedIntensity, 
                pointLightDir, 
                viewDir, 
                Normal, 
                Specular
            );

            float shadow = GetPointLightDataShadow(
                pointLight.shadowCubemapIndex, 
                FragPos, 
                pointLight.position, 
                pointLight.shadowFarPlane
            );
            
		    lightingResult += (1.0 - shadow) * pointLightResult;
	    };
    };

    // directional light
    if (hasDirectionalLight) {
        vec3 lightDir = normalize(-directionalLightDirection);
        
        vec3 directionalLighting = GetLight(
            directionalLightColor, 
            directionalLightIntensity, 
            lightDir, 
            viewDir, 
            Normal, 
            Specular
        );

        // float shadowBias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);
        // float shadow = GetDirectLightShadow(FragPos, shadowBias);
        float chebysev_shadow = GetChebysevShadow(FragPos);
        lightingResult += (1-chebysev_shadow) * directionalLighting; // only apply shadow to the directional light for now, point & spot lights later
    };

    // ambient light
    vec3 ambientLight = ambientLightColor * ambientLightIntensity;

    lightingResult = lightingResult + ambientLight;

    vec3 result = lightingResult * Albedo;
    FragColor = vec4(result, 1.0);
    
};

vec3 GetLight(vec3 lightColor, float lightIntensity, vec3 lightDir, vec3 viewDir, vec3 normal, float specular) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 light = lightColor * lightIntensity;

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light * diff;
    
    // specular
    float specFactor = pow(max(dot(normal, halfwayDir), 0.0), SHININESS); // blinn-phong
    vec3 spec = specFactor * specular * light;

    return diffuse + spec;;
};

float GetRandomPoissonIndex(vec4 seed4) {
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
};

float GetPointLightDataShadow(int i, vec3 fragPos, vec3 lightPos, float farPlane) {
    float shadow  = 0.0;
    float bias    = 0.05; 
    float samples = 20.0;
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float viewDistance = length(viewPos.xyz - fragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;

    for(int j = 0; j < samples; ++j)
    {
        vec3 nudge = fragToLight + sampleOffsetDirections[j] * diskRadius;
        float closestDepth = texture(shadowCubemapArray, vec4(nudge, i)).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    shadow /= float(samples);  
    return shadow;
};

float GetDirectLightShadow(vec3 fragPos, float bias) {
    vec4 fragPosLightSpace = directionalLightMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;
    
    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(directionalLightShadowMap, 0);
    float shadow = 0.0;

    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y < 1; ++y) {
            
            //float pcfDepth = texture(directionalLightShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;

            // poisson sampling
            float nSampledShadow = 0.0;
            for (int i = 0; i < 16; i++) {
                int index = int(16.0 * GetRandomPoissonIndex(vec4(fragPos * 1000.0, i))) % 16;
                float poissonDepth = texture(directionalLightShadowMap, projCoords.xy + vec2(x, y) * texelSize + POISSON_SPHERE_16[index].xy/1600.0).r;
                nSampledShadow += currentDepth - bias > poissonDepth? 1.0 : 0.0;
            };

            shadow += nSampledShadow / 16.0;
            //shadow += currentDepth - bias > pcfDepth? 1.0 / 9.0 : 0.0;
        };
    };
    
    shadow /= 9.0; // take the average from the 9 sampled points around it

    return shadow;
};

float ChebysevUpperBound(vec4 moments, float currentDepth) {
    float mean = moments.r;
    float variance = max(moments.g - mean * mean, 0.01);
    float skewness = moments.b - 3.0 * mean * variance - mean * mean * mean;
    float kurtosis = moments.a - 4.0 * mean * skewness - 6.0 * mean * mean * variance - mean * mean * mean * mean;

    float mD = currentDepth - mean;
    float mD_2 = mD * mD;

    float p = variance / (variance + mD_2);
    
    float correction = 1.0;
    if (skewness > 0)
        correction -= skewness * 0.1;

    if (kurtosis > 0)
        correction *= 1.0 / (1.0 + kurtosis * 0.01);

    p = clamp(correction * p, 0, 1);
    p = smoothstep(0, 1, p);
    return p;
};

float GetChebysevShadow(vec3 fragPos) {
    vec4 fragPosLightSpace = directionalLightMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    int n_samples = 16;
    float bias = 0.1;

    vec2 texelSize = 1 / vec2(textureSize(directionalLightShadowMap, 0)); // Shadow map texel size
    float shadow = 0;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y < 1; ++y) {
            
            // poisson sampling
            float nSampledShadow = 0.0;
            for (int i = 0; i < n_samples; i++) {
                int index = int(n_samples * GetRandomPoissonIndex(vec4(fragPos * 1000.0, i))) % n_samples;
                vec4 moments = texture(directionalLightShadowMap, projCoords.xy + vec2(x, y) * texelSize + POISSON_SPHERE_16[index].xy/200.0);

                float shadow_p = ChebysevUpperBound(moments, currentDepth + 0.005); 
                if (currentDepth < moments.x)
                    nSampledShadow += 1.0;

                if (shadow_p > 0.5)
                    nSampledShadow += 1.0;
                else
                    nSampledShadow += 0;
            };

            shadow += nSampledShadow / n_samples;
        };
    };

    shadow /= 9;
    return shadow;
};