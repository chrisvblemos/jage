#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <memory>
#include "../EntityManager.h"
#include "../GameAsset.h"

#define OPENGL_MAX_POINT_LIGHTS 32
#define LIGHTS_UNIFORM_BUFFER_INDEX 0
#define CAMERA_UNIFORM_BUFFER_INDEX 1

struct Texture;
struct Shader;
struct Material;
struct StaticMesh;
struct Camera;
struct Transform;
struct DirectionalLight;
struct PointLight;

struct PointLightUniformData {
	glm::vec4 mPositionAndRadius{ 0.0f };
	glm::vec4 mColorAndIntensity{ 1.0f };
};

struct DirectionalLightUniformData {
	glm::vec4 mDirection{ 0.0f };
	glm::vec4 mColorAndIntensity{ 1.0f };
};


struct LightsUniformBlock {
	DirectionalLightUniformData mDirectionalLight;
	PointLightUniformData mPointLights[OPENGL_MAX_POINT_LIGHTS];
	glm::vec4 mAmbientLightColorAndIntensity{ 1.0f };
	uint32_t mPointLightsCount;
};

struct CameraUniformBlock {
	glm::vec4 mPosition{ 0.0f };
	glm::mat4 mViewMat{ 1.0f };
	glm::mat4 mProjMat{ 1.0f };
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
	uint32_t SCR_WIDTH = 800;
	uint32_t SCR_HEIGHT = 600;
	uint32_t SHADOW_MAP_WIDTH = 4096;
	uint32_t SHADOW_MAP_HEIGHT = 4096;

	OpenGL() = default;

	uint32_t lightsUniformBufferObject = 0;
	uint32_t cameraUniformBufferObject = 0;

	uint32_t screenFrameBufferObject= 0;
	uint32_t screenRenderBufferObject = 0;
	uint32_t screenQuadVertexArrayObject = 0;
	uint32_t screenQuadVertexBufferObject = 0;
	uint32_t screenFramebufferTextureId = 0;

	uint32_t gBuffer;
	uint32_t gPosition, gNormal, gAlbedoSpec, gShadowMap;
	uint32_t gDepth;

	uint32_t shadowMapFrameBufferObject;
	uint32_t shadowMapTextureId;

	// cpu data
	DirectionalLight* dirLight;
	std::vector<PointLight*> pointLights;
	Camera* currentActiveCamera;

	std::unordered_map<uint32_t, StaticMesh*> meshVertexArrayObjToStaticMeshMap;
	std::unordered_map<uint32_t, Transform*> instanceIdToTransformMap;
	std::unordered_map<uint32_t, std::vector<Entity>> meshVertexArrayObjToInstanceIdMap;

	std::unordered_map<ASSET_ID, uint32_t> loadedTexturesTable{}; // currently gpu loaded textures // loadedTextures[assetId] = id;

	void InitGBuffer();
	void InitShadowMap();

	void InitCameraUniformBufferObject();
	void InitLightsUniformBufferObject();

	void BufferScreenQuad();

	void CompileShader(Shader& shader);
	void UseShader(Shader* shader);
	void SetShaderBool(Shader* shader, const std::string& name, const bool value);
	void SetShaderInt(Shader* shader, const std::string& name, const int value);
	void SetShaderFloat(Shader* shader, const std::string& name, const float value);
	void SetShaderMat4(Shader* shader, const std::string& name, const glm::mat4& value);
	void SetShaderVec3(Shader* shader, const std::string& name, const glm::vec3& value);
	void SetShaderVec2(Shader* shader, const std::string& name, const glm::vec2& value);

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
		dirLight = light;
	};

	void RegisterCamera(Camera* camera) {
		if (camera != nullptr) {
			currentActiveCamera = camera;
		}
	}

	// Uniform buffer objects
	void UploadLightUniforms();
	void UploadCameraUniforms();

	// Buffering
	void BufferTexture(Texture* texture);
	void BufferStaticMesh(Entity instanceId, StaticMesh* staticMesh, Transform* transform);
	
	// Passes
	void GeometryPass();
	void ShadowMapPass();
	void LightPass();

	void DebugGbuffer(uint32_t layer = 0);
};