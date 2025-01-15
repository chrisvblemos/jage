#pragma once

#include <Engine/Renderer/Types/Types.h>
#include <Engine/Core.h>

struct Texture;
struct Transform;
struct Material;
struct StaticMesh;
struct Camera;
struct DirectionalLight;
struct PointLight;

// UBOs
#define LIGHTS_UNIFORM_BUFFER_INDEX 0
#define CAMERA_UNIFORM_BUFFER_INDEX 1

// SSBOs
#define SSBO_MESH_INSTANCE_DATA 0
#define SSBO_MATERIAL_INSTANCE_DATA 1
#define SSBO_POINT_LIGHT_DATA_ARRAY 2

// SHADERS
#define SHADER_LIGHTING 0
#define SHADER_SCREEN 1
#define SHADER_GBUFFER 2
#define SHADER_SHADOW_MAP 3
#define SHADER_POINT_SHADOW_MAP 4

// MESHES
#define MAX_MESH_INSTANCES 10000
#define MAX_MATERIAL_INSTANCES 50000
#define MAX_POINT_LIGHTS 32

struct MaterialData {
	glm::vec4 baseColor;
	glm::vec4 specularColor;
	float shininess;
	float roughness;
	uint32_t textureIndices[3];
	float padding[2];
};

struct MeshData {
	uint32_t firstIndex;		// location in the EBO
	uint32_t indexCount;		// number of indices
	uint32_t baseVertex;		// location in the VBO
	uint32_t drawCommandId;		// index in draw command buffer
};

struct MeshInstanceData {
	glm::mat4 model;
	glm::mat4 inverseModel;
	uint32_t id;
	uint32_t materialIndex;
};

struct SceneLightData {
	bool mHasDirectionalLight;
    glm::vec3 mDirectionalLightDirection;
    glm::vec3 mDirectionalLightColor;
    float mDirectionalLightIntensity;
    glm::mat4 mDirectionalLightMatrix;
    int mPointLightsCount;
    glm::vec3 mAmbientLightColor;
    float mAmbientLightIntensity;
};

struct PointLightData {
	glm::vec3 mPosition;
	float mRadius;
	glm::vec3 mColor;
	float mIntensity;
	float shadowFarPlane;
	int shadowCubeMapIndex;
	float padding;
};

struct PointLightsDataArray {
	PointLightData mPointLights[MAX_POINT_LIGHTS];
};

struct CameraData {
	glm::vec4 mPosition;
	glm::mat4 mProjection{ 1.0f };
	glm::mat4 mView{ 1.0f };
};

// screen quad
const float quadVertices[] = {
	// Positions   // Texture Coords
	-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
	-1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
	 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right

	-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
	 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
	 1.0f,  1.0f,  1.0f, 1.0f   // Top-right
};

class OpenGL {
private:
	OpenGL() = default;

	uint32_t SCR_WIDTH = 800;
	uint32_t SCR_HEIGHT = 600;

	uint32_t SHADOW_MAP_RESOLUTION = 512;

	std::unordered_map<uint32_t, Shader> mCompiledShaders;

	UniformBuffer sceneLightDataUBO;
	UniformBuffer cameraDataUBO;

	std::unordered_map<uint32_t, MeshData> meshAssetToDataMap;
	std::vector<MeshInstanceData> meshInstanceDataArray;
	std::vector<DrawIndirectElementCommand> meshDrawCommandArray;

	std::vector<MaterialData> materialDataArray;

	DrawIndirectBuffer meshDIB;
	VertexArrayBuffer meshVAO;
	VertexBuffer meshVBO;
	ElementArrayBuffer meshEBO;
	ShaderStorageBuffer meshSSBO;
	ShaderStorageBuffer materialSSBO;

	FrameBuffer screenFBO;
	uint32_t screenRBO = 0;
	VertexArrayBuffer screenQuadVAO;
	VertexBuffer screenQuadVBO;
	Texture2D screenTextureID;

	FrameBuffer gBuffer;
	Texture2D gPosition, gNormal, gAlbedoSpec, gShadowMap, gDepth;

	FrameBuffer shadowMapFBO;
	Texture2D directionalLightShadowMap;

	FrameBuffer pointShadowFBO;
	ShaderStorageBuffer pointLightDataArraySSBO;
	TextureCubeMapArray pointShadowCubemapArray;

	// CPU data
	DirectionalLight* directionalLight;
	Camera* currentActiveCamera;
	std::vector<uint32_t> pointShadowCubemaps;
	std::vector<PointLight*> pointLights;
	std::unordered_map<uint32_t, StaticMesh*> VAOToMeshMap;
	std::unordered_map<uint32_t, Transform*> meshInstanceToTransformMap;
	std::unordered_map<uint32_t, std::vector<Entity>> VAOToInstanceIdMap;
	std::unordered_map<AssetId, uint32_t> assetToTextureIDMap{}; // loadedTextures[assetId] = gl_id;

	void InitGBuffer();
	void InitShadowMap();

	void BufferScreenQuad();

	glm::mat4 CalculateModelMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

public:
	// prevents copying
	OpenGL(const OpenGL&) = delete;
	OpenGL& operator=(const OpenGL&) = delete;

	static OpenGL& Get() {
		static OpenGL instance;
		return instance;
	}

	bool Initialize();

	// Data loading
	void RegisterPointLight(PointLight* light) {
		if (light != nullptr) {
			pointLights.push_back(light);
		}
	};

	void RegisterDirectionalLight(DirectionalLight* light) {
		directionalLight = light;
	};

	void RegisterCamera(Camera* camera) {
		if (camera != nullptr) {
			currentActiveCamera = camera;
		}
	}

	// Uniform buffer objects
	void UploadSceneLightData();
	void UploadCameraData();

	// Buffering
	void BufferTexture(Texture* texture);
	void BufferStaticMesh(Entity instanceId, StaticMesh* staticMesh, Transform* transform);
	
	// Passes
	void GeometryPass();
	void ShadowMapPass();
	void PointShadowMapPass();
	void LightingPass();

	void DebugGbuffer(uint32_t layer = 0);
};