#version 460 core

const int MAX_POINT_LIGHTS = 32;
const float SHININESS = 32.0;

const vec2 POISSON_DISK_16[16] = vec2[](
        vec2(0.45185273, 0.26027307),
        vec2(0.65157867, 0.06379210),
        vec2(0.34067438, 0.88627897),
        vec2(0.24379788, 0.70906218),
        vec2(0.77983632, 0.11903441),
        vec2(0.07128592, 0.46912458),
        vec2(0.73672142, 0.85098227),
        vec2(0.84263736, 0.56780259),
        vec2(0.95098022, 0.65227214),
        vec2(0.93549333, 0.33306261),
        vec2(0.75305322, 0.75939346),
        vec2(0.59904599, 0.57617012),
        vec2(0.14080315, 0.69091085),
        vec2(0.74251429, 0.53334101),
        vec2(0.63054442, 0.89339291),
        vec2(0.61337256, 0.74964296)
);

struct PointLight {
    vec4 positionAndRadius;
	vec4 colorAndIntensity;
};

struct DirectionalLight {
    vec4 direction;
    vec4 colorAndIntensity;
};

layout(std140, binding = 0) uniform LightsBlock {
    DirectionalLight uDirectionalLight;
    PointLight uPointLights[MAX_POINT_LIGHTS];
    vec4 uAmbientLightColorAndIntensity;
    int uPointLightsCount;
};

layout(std140, binding = 1) uniform CameraBlock {
    vec4 uCamViewPos;
};

vec3 CalcLight(vec3 lightColor, float lightIntensity, vec3 lightDir, vec3 viewDir, vec3 normal, float specular);
float CalcShadowMapVal(vec3 fragPos, float bias);
float RandomPoissonIndex(vec4 seed4);

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D uShadowMap;
uniform mat4 uLightSpaceMatrix;

out vec4 FragColor;

void main() {
	vec3 FragPos = texture(gPosition, TexCoords).rgb;       // gPosition texture
	vec3 Normal = texture(gNormal, TexCoords).rgb;          // gNormal texture
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;      // gAlbedo texture
	float Specular = texture(gAlbedoSpec, TexCoords).a;     // gAlbedoSpec texture (stored in alpha of gAlbedo)
    
    vec3 viewDir = normalize(uCamViewPos.xyz - FragPos); 
    vec3 lightingResult = vec3(0.0, 0.0, 0.0);

    // point lights
    if (uPointLightsCount > 0) {
        int maxPointLightsIndex = min(uPointLightsCount, MAX_POINT_LIGHTS); // avoid out of bounds case
	    for (int i = 0; i < maxPointLightsIndex; ++i) {
            PointLight pointLight = uPointLights[i];
            vec3 pointLightPos = pointLight.positionAndRadius.xyz;
            float pointLightRadius = pointLight.positionAndRadius.w;
		    vec3 pointLightColor = pointLight.colorAndIntensity.xyz;
            float pointLightIntensity = pointLight.colorAndIntensity.w;

            vec3 pointLightDir = normalize(pointLightPos - FragPos);
            float attenuatedIntensity = pointLightIntensity / pow(length(pointLightPos - FragPos), 2);

            vec3 pointLightResult = CalcLight(pointLightColor, attenuatedIntensity, pointLightDir, viewDir, Normal, Specular);
		    lightingResult += pointLightResult;
	    }
    }

    // directional light
    if (uDirectionalLight.colorAndIntensity.w > 0.0) {
        vec3 lightColor = uDirectionalLight.colorAndIntensity.xyz;
        float lightIntensity = uDirectionalLight.colorAndIntensity.w;
        vec3 lightDir = normalize(-uDirectionalLight.direction.xyz);
        vec3 directionalLighting = CalcLight(lightColor, lightIntensity, lightDir, viewDir, Normal, Specular);

        float bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);
        float shadow = CalcShadowMapVal(FragPos, bias);

        lightingResult += (1.0 - shadow) * directionalLighting; // only apply shadow to the directional light for now, point & spot lights later
    }

    // ambient light
    vec3 ambientLightColor = uAmbientLightColorAndIntensity.xyz;
    float ambientLightIntensity = uAmbientLightColorAndIntensity.w;
    vec3 ambientLight = ambientLightColor * ambientLightIntensity;

    lightingResult = lightingResult + 0.1 * ambientLight; // this is the resulting light value map

    vec3 result = lightingResult * Albedo;// apply resulting light map to object surface
    FragColor = vec4(result, 1.0);
}

vec3 CalcLight(vec3 lightColor, float lightIntensity, vec3 lightDir, vec3 viewDir, vec3 normal, float specular) {
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

float RandomPoissonIndex(vec4 seed4) {
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float CalcShadowMapVal(vec3 fragPos, float bias) {
    vec4 fragPosLightSpace = uLightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;
    
    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float shadow = 0.0;

    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y < 1; ++y) {
            
            //float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;

            // poisson sampling
            float nSampledShadow = 0.0;
            for (int i = 0; i < 16; i++) {
                int index = int(16.0 * RandomPoissonIndex(vec4(fragPos * 1000.0, i))) % 16;
                float poissonDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize + POISSON_DISK_16[index]/1600.0).r;
                nSampledShadow += currentDepth - bias > poissonDepth? 1.0 : 0.0;
            }

            shadow += nSampledShadow / 16.0;
            //shadow += currentDepth - bias > pcfDepth? 1.0 / 9.0 : 0.0;
        }
    }
    
    shadow /= 9.0; // take the average from the 9 sampled points around it

    return shadow;
}