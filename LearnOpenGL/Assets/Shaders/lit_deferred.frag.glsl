#version 460 core

const int MAX_POINT_LIGHTS = 32;
const float SHININESS = 32.0;

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

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 halfDir);

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

out vec4 FragColor;

void main() {
	vec3 FragPos = texture(gPosition, TexCoords).rgb;       // gPosition texture
	vec3 Normal = texture(gNormal, TexCoords).rgb;          // gNormal texture
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;      // gAlbedo texture
	float Specular = texture(gAlbedoSpec, TexCoords).a;     // gAlbedoSpec texture (stored in alpha of gAlbedo)
	
    vec3 result = vec3(0.0); // start with no ambient light

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
		    vec3 diffuse = max(dot(Normal, pointLightDir), 0.0) * Albedo * pointLightColor * pointLightIntensity;
		    result += diffuse;
	    }
    }

    // directional light (ie Sun)
    if (uDirectionalLight.colorAndIntensity.w > 0.0) {
        vec3 dirLight = normalize(-uDirectionalLight.direction.xyz);
        vec3 norm = texture(gNormal, TexCoords).rgb;
	    vec3 viewDir = normalize(uCamViewPos.xyz - FragPos); 
        vec3 halfwayDir = normalize(dirLight + viewDir);
        result += CalcDirLight(uDirectionalLight, norm, viewDir, halfwayDir);
    }
    else {
        // if no directional light, use ambient light
        vec3 ambientLightColor = uAmbientLightColorAndIntensity.xyz;
        float ambientLightIntensity = uAmbientLightColorAndIntensity.w;
        result += Albedo * ambientLightColor * ambientLightIntensity;
    }

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 halfDir) {
    vec3 lightColor = light.colorAndIntensity.xyz;
    float lightIntensity = light.colorAndIntensity.w;
    vec3 lightDir = normalize(-light.direction.xyz);
    vec3 reflectDir = reflect(-lightDir, normal);

    // diffuse
    vec3 baseColor = vec3(texture(gAlbedoSpec, TexCoords).rgb);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * lightIntensity * baseColor * diff;
    
    // specular
    float specFactor = pow(max(dot(normal, halfDir), 0.0), SHININESS); // blinn-phong
    vec3 specColor = vec3(texture(gAlbedoSpec, TexCoords).r);
    vec3 specular = lightColor * lightIntensity * specColor * specFactor;

    return diffuse + specular;
};