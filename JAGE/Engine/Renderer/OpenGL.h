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
#define UBO_SHADOW_CASCADE_DATA 8
#define UBO_SSAO_SETTINGS_DATA 9

// MESHES
#define MAX_MESHES 10000
#define MAX_MESH_INSTANCES 20000
#define MAX_MATERIAL_INSTANCES 200
#define MAX_POINT_LIGHTS 128
#define MAX_TEXTURES 10000

#define SHADOW_MAP_RESOLUTION 1024
#define SHADOW_MAP_N_CASCADES 4
#define SHADOW_MAP_MAX_CASCADES 16

#define SSAO_KERNEL_SIZE 64
#define SSAO_RADIUS 1.8f
#define SSAO_BIAS 0.025f
#define SSAO_POWER .5f

#define GAMMA 2.2f

enum SceneTexture : uint32_t {
	ST_POST_FX,
	ST_DIFFUSE,
	ST_SSAO,
	ST_ALBEDO,
	ST_SPECULAR,
	ST_METALLIC,
	ST_DEPTH,
	ST_WORLD_NORMAL,
	ST_WORLD_POSITION,
	ST_VIEW_POSITION,
	ST_VIEW_NORMAL,
	COUNT
};

struct SSAOSettingsData {
	GLuint kernelSize = SSAO_KERNEL_SIZE;
	float radius = SSAO_RADIUS;
	float padding0[2];
	glm::vec4 samples[SSAO_KERNEL_SIZE];
	glm::vec2 noiseScale;
	float bias = SSAO_BIAS;
	float power = SSAO_POWER;
};
 
struct CascadeData {
	glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
	float	  farPlane  = 0.0f;
	float	  nearPlane = 0.0f;
	float	  padding[2];
};

struct MeshDrawCmdData {
	GLuint count         = 0;	// n of indices to draw for each instance
	GLuint instanceCount = 0;	// n of instances to draw
	GLuint firstIndex    = 0;	// index of start of indices
	GLuint baseVertex    = 0;	// index of start of vertex
	GLuint baseInstance  = 0;	// index of first instance

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
	GLuint	  mHasDirectionalLight;
	float	  padding1[3];
    glm::vec3 mDirectionalLightDirection;
	float	  padding2;
    glm::vec3 mDirectionalLightColor;
    float	  mDirectionalLightIntensity;
    glm::mat4 mDirectionalLightMatrix;
	GLuint	  mPointLightsCount;
	float	  padding3[3];
    glm::vec3 mAmbientLightColor;
    float	  mAmbientLightIntensity;
};

struct PointLightData {
	glm::vec3 mPosition;
	float	  mRadius;
	glm::vec3 mColor;
	float	  mIntensity;
	float	  shadowFarPlane;
	float	  shadowNearPlane;
	GLuint	  shadowCubeMapIndex;
	GLuint	  dataArrayIndex;
	float	  constant = 1.0f;
	float	  linear = 0.7f;
	float	  quadratic = 1.8f;
	float	  padding[3];
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

	Shader lightingShader;
	Shader screenShader;
	Shader gBufferShader;
	Shader pointLightShadowMapShader;
	Shader varianceShadowMapShader;
	Shader hBlurShadowMapShader;
	Shader vBlurShadowMapShader;
	Shader shadowMapShader;
	Shader ssaoShader;
	Shader ssaoBlurShader;
	Shader postFxShader;
	
	UniformBuffer sceneLightDataUBO;
	UniformBuffer cameraDataUBO;
	UniformBuffer cascadeDataUBO;
	UniformBuffer ssaoSettingsDataUBO;

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
	FrameBuffer gBufferFBO;
	FrameBuffer shadowMapFBO;
	FrameBuffer pointShadowFBO;
	FrameBuffer hBlurShadowMapFBO;
	FrameBuffer vBlurShadowMapFBO;
	FrameBuffer lightingFBO;
	FrameBuffer postfxFBO;
	FrameBuffer ssaoFBO;
	FrameBuffer ssaoBlurFBO;

	/* gBuffer textures */
	Texture2D gPosition;
	Texture2D gNormal;
	Texture2D gAlbedo;
	Texture2D gSpecular;
	Texture2D gMetallic;
	Texture2D gViewPosition;
	Texture2D gViewNormal;
	Texture2D gDepth;
	
	/* Shadow maps */
	Texture2DArray shadowMapTex2DArray;
	TextureCubeMapArray pointShadowCubemapArray;
	Texture2D vBlurShadowMapTex2D;
	Texture2D hBlurShadowMapTex2D;

	/* Output texts */
	Texture2D SSAOTex2D;
	Texture2D SSAONoiseTex2D;
	Texture2D SSAOBlurTex2D;
	Texture2D diffuseTex2D;
	Texture2D postFXTex2D;

	// CPU data
	DirectionalLight* directionalLight;
	Camera* currentActiveCamera;

	std::vector<CascadeData> cascadeDataArray;
	
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
	void InitShadowMapFBOs();
	void InitLightingFBO();
    void InitPostFxFBO();
	void InitSSAOUniformBuffer();

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

    void RegisterDirectionalLight(DirectionalLight *light)
    {
        directionalLight = light;
    };

    void RegisterCamera(Camera* camera);

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
	void SSAOPass();
	void ShadowMapPass();
	void PointLightShadowMapPass();
	void LightingPass();
	void PostFxPass();
    void OutputToScreen(const SceneTexture st);


    void DebugGbuffer(uint32_t layer = 0);
};