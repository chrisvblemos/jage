#pragma once

#include "Types/Buffer.h"
#include "Types/Shader.h"
#include "Types/FrameBuffer.h"
#include "Types/Texture.h"
#include "Types/VertexArray.h"
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
#define MAX_POINT_LIGHTS 128
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
	float shadowNearPlane;
	uint32_t shadowCubeMapIndex;
	uint32_t dataArrayIndex;
	float padding[2];
};

struct CameraData {
	glm::vec4 mPosition;
	glm::mat4 mProjection{ 1.0f };
	glm::mat4 mView{ 1.0f };
};

// screen quad
const float quadVertices[] = {
	// Positions   // Texture coordinates
	-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
	-1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
	 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right

	-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
	 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
	 1.0f,  1.0f,  1.0f, 1.0f   // Top-right
};

const GLuint quadIndices[] = {
	0, 1, 2,  // First Triangle: Top-left, Bottom-left, Bottom-right
	3, 4, 5   // Second Triangle: Top-left, Bottom-right, Top-right
};

class OpenGL {
private:
	OpenGL() = default;

	uint32_t SHADOW_MAP_RESOLUTION = 512;

	Shader lightingShader;
	Shader screenShader;
	Shader gBufferShader;
	Shader shadowMapShader;
	Shader pointShadowMapShader;

	UniformBuffer sceneLightDataUBO;
	UniformBuffer cameraDataUBO;

	DrawIndirectBuffer meshDIB;

	VertexArray meshVAO;
	VertexArray screenQuadVAO;

	VertexArrayBuffer meshVBO;
	VertexArrayBuffer screenQuadVBO;

	ElementArrayBuffer screenQuadEBO;
	ElementArrayBuffer meshEBO;

	ShaderStorageBuffer meshSSBO;
	ShaderStorageBuffer textureHndlrsSSBO;
	ShaderStorageBuffer pointLightDataArraySSBO;

	FrameBuffer screenFBO;
	FrameBuffer gBuffer;
	FrameBuffer shadowMapFBO;
	FrameBuffer pointShadowFBO;
	
	Texture2D screenTextureID;
	Texture2D gPosition;
	Texture2D gNormal;
	Texture2D gAlbedoSpec;
	Texture2D gShadowMap;
	Texture2D gDepth;
	Texture2D directionalLightShadowMap;

	TextureCubeMapArray pointShadowCubemapArray;

	// CPU data
	DirectionalLight* directionalLight;
	Camera* currentActiveCamera;
	
	std::unordered_map<uint32_t, Shader> mCompiledShaders;
	std::unordered_map<AssetId, std::unordered_map<Entity, uint32_t>> assToEntMeshInstIndexes;
	std::unordered_map<AssetId, MeshMetaData> assToMesh;
	std::unordered_map<AssetId, std::vector<MeshInstanceData>> assToMeshInsts;
	std::unordered_map<Entity, PointLightData> entToPointLightData;

	std::vector<MeshDrawCmdData> meshDrawCmdDataArray;
	std::vector<GLuint> meshIndexDataArray;
	std::vector<Vertex> meshVertexDataArray;
	std::vector<uint32_t> depthCubemapDataArray;
	std::vector<PointLightData> pointLightDataArray;

	std::unordered_map<AssetId, Texture2D> assetIDToTex2DMap;
	std::vector<GLuint64> tex2DHndlrDataArray;

	void InitGBuffer();
	void InitShadowMap();

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

	void RegisterDirectionalLight(DirectionalLight* light) {
		directionalLight = light;
	};

	void RegisterCamera(Camera* camera) {
		if (camera != nullptr) {
			currentActiveCamera = camera;
		}
	}

	void RegisterPointLight(const Entity entity, const PointLight* light);
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