#pragma once

#include <Renderer/Types/Types.h>
#include <Core/Core.h>

struct Vertex;
struct Texture;
struct Transform;
struct Material;
struct Mesh;
struct Camera;
struct DirectionalLight;
struct PointLight;
struct StaticMeshRenderer;

// UBOs & SSBOs
#define UBO_LIGHTS 0
#define UBO_CAMERA_DATA 1
#define SSBO_MESH_INSTANCE_DATA 2
#define SSBO_MATERIAL_INSTANCE_DATA 3
#define SSBO_POINT_LIGHT_DATA_ARRAY 4
#define UBO_SCENE_LIGHT_DATA 5
#define SSBO_TEXTURE_HANDLERS 6
#define SSBO_MESH_INDIRECT_DRAW_COMMAND 7

// SHADERS
#define SHADER_LIGHTING 1
#define SHADER_SCREEN 2
#define SHADER_GBUFFER 3
#define SHADER_SHADOW_MAP 4
#define SHADER_POINT_SHADOW_MAP 5

// MESHES
#define MAX_MESHES 10000
#define MAX_MESH_INSTANCES 20000
#define MAX_MATERIAL_INSTANCES 200
#define MAX_POINT_LIGHTS 32
#define MAX_TEXTURES 10000

struct MeshDrawCmdData {
	GLuint count = 0;				// n of indices to draw for each instance
	GLuint instanceCount = 0;		// n of instances to draw
	GLuint firstIndex = 0;			// index of start of indices
	GLuint baseVertex = 0;			// index of start of vertex
	GLuint baseInstance = 0;			// index of first instance

	GLint diffTexHndlrIndex = -1;
	GLint specTexHndlrIndex = -1;
	GLint normTexHndlrIndex = -1;
};


struct MeshInstanceData {
	glm::mat4 model;
	glm::mat4 inverseModel;
};

struct MeshMetaData {
	int32_t drawCmdIndex = -1;
};

struct SceneLightData {
	uint32_t mHasDirectionalLight;
	float padding1[3];
    glm::vec3 mDirectionalLightDirection;
	float padding2;
    glm::vec3 mDirectionalLightColor;
    float mDirectionalLightIntensity;
    glm::mat4 mDirectionalLightMatrix;
	uint32_t mPointLightsCount;
    glm::vec3 mAmbientLightColor;
    float mAmbientLightIntensity;
};

struct PointLightData {
	glm::vec3 mPosition;
	float mRadius;
	glm::vec3 mColor;
	float mIntensity;
	float shadowFarPlane;
	uint32_t shadowCubeMapIndex;
	float padding[2];
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

	uint32_t SHADOW_MAP_RESOLUTION = 512;

	std::unordered_map<uint32_t, Shader> mCompiledShaders;

	UniformBuffer sceneLightDataUBO;
	UniformBuffer cameraDataUBO;

	DrawIndirectBuffer meshDIB;
	VertexArrayBuffer meshVAO;
	VertexBuffer meshVBO;
	ElementArrayBuffer meshEBO;
	ShaderStorageBuffer meshSSBO;
	ShaderStorageBuffer textureHndlrsSSBO;

	std::unordered_map<AssetId, std::unordered_map<Entity, uint32_t>> assToEntMeshInstIndexes;
	std::unordered_map<AssetId, MeshMetaData> assToMesh;
	std::unordered_map<AssetId, std::vector<MeshInstanceData>> assToMeshInsts;

	std::vector<MeshDrawCmdData> meshDrawCmdDataArray;
	std::vector<GLuint> meshIndexDataArray;
	std::vector<Vertex> meshVertexDataArray;

	std::unordered_map<AssetId, Texture2D> assetIDToTex2DMap;
	std::vector<GLuint64> tex2DHndlrDataArray;

	FrameBuffer screenFBO;
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

	void UploadSceneLightData();
	void UploadCameraData();
	void BatchMeshInstData();

	// Buffering
	void RegisterTexture2D(Texture* texture);
	void UpsertMeshEntity(const Entity entity, const std::vector<const Mesh*>& meshes, const Transform& transform);
	void RegisterMesh(const Mesh* mesh);
	
	// Passes
	void GeometryPass();
	void ShadowMapPass();
	void PointShadowMapPass();
	void LightingPass();

	void DebugGbuffer(uint32_t layer = 0);
};