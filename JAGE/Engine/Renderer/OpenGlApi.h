#pragma once

#include "Types/Buffer.h"
#include "Types/Shader.h"
#include "Types/FrameBuffer.h"
#include "Types/Texture.h"
#include "Types/VertexArray.h"
#include <Core/Core.h>
#include <Core/Config.h>

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

#define SHADOW_MAP_MAX_CASCADES 16
#define SSAO_KERNEL_SIZE 64

struct Vertex;
struct Texture;
struct Transform;
struct Material;
struct Mesh;
struct Camera;
struct DirectionalLight;
struct PointLight;
struct StaticMeshRenderer;

struct SSAOSettingsData {
	GLuint    kernelSize;
	float     radius;
	float     padding0[2];
	glm::vec4 samples[SSAO_KERNEL_SIZE];
	glm::vec2 noiseScale;
	float     bias;
	float     power;
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
	float padding;
	glm::vec3 mColor;
	float	  mIntensity;
	float	  shadowFarPlane;
	float	  shadowNearPlane;
	GLuint	  shadowCubeMapIndex;
	GLuint	  dataArrayIndex;
};

struct CameraData {
	glm::vec4 mPosition;
	glm::mat4 mProjection{ 1.0f };
	glm::mat4 mView{ 1.0f };
};


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

struct Viewport {
	GLuint x_offset = 0;
	GLuint y_offset = 0;
	GLuint width = 800;
	GLuint height = 600;
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


class OpenGlApi {
private:
	OpenGlApi() = default;

	Viewport mViewport;
	bool mDepthEnabled;
	bool mFaceCullEnabled;
	GLenum mFaceCullMode;

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
	
	std::unordered_map<Asset, std::unordered_map<Entity, uint32_t>> assToEntMeshInstIndexes;
	std::unordered_map<Asset, MeshMetaData> assToMesh;
	std::unordered_map<Asset, std::vector<MeshInstanceData>> assToMeshInsts;
	std::unordered_map<Entity, PointLightData> entToPointLightData;

	std::vector<CascadeData> cascadeDataArray;

	std::vector<MeshDrawCmdData> meshDrawCmdDataArray;
	std::vector<GLuint> meshIndexDataArray;
	std::vector<Vertex> meshVertexDataArray;
	std::vector<uint32_t> depthCubemapDataArray;
	std::vector<PointLightData> pointLightDataArray;

	std::unordered_map<Asset, Texture2D> assetIDToTex2DMap;
	std::vector<GLuint64> tex2DHndlrDataArray;

	void SetDepthEnabled(const bool val);
	void SetFaceCullEnabled(const bool val);
	void SetFaceCullMode(const GLenum mode);
	void SetViewport(const GLint x_off, const GLint y_off, const GLsizei width, const GLsizei height);
	void ClearColor(const glm::vec4& rgba);
	void ClearBuffers(const GLenum flags);

	GLenum internalToFormat(const GLenum internalFormat);

	void InitGBuffer();
	void InitShadowMapFBOs();
	void InitLightingFBO();
	void InitPostFxFBO();
    void InitSSAOUniformBuffer();
    
    std::vector<glm::vec4> GetFrustumCornersWorldSpace(const float fov, const float aspectRatio, const float nearPlane, const float farPlane, const glm::mat4 &view);
    glm::mat4 GetLightSpaceMatrix(const glm::mat4 &lightViewMatrix, const std::vector<glm::vec4> &corners);
    glm::mat4 GetLightViewMatrix(const glm::vec3 &lightDir, const std::vector<glm::vec4> &frustumCorners);
    glm::mat4 CalculateModelMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

public:
	OpenGlApi(const OpenGlApi&) = delete;
	OpenGlApi& operator=(const OpenGlApi&) = delete;

	static OpenGlApi& Get() {
		static OpenGlApi instance;
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


    void DrawScreenQuad(Texture2D& texture);
    void DrawScene(const bool withDepth, const GLenum faceCulling, const bool clear = true);
};