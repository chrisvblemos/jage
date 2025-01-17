#include <ECS/EntityManager.h>
#include <Core/Types/GameAsset.h>
#include <Core/Types/Texture.h>
#include <Core/Types/StaticMesh.h>
#include <Core/Types/Material.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Components/PointLight.h>
#include <Utils.h>
#include <LogDisplay.h>

#include "OpenGLUtils.h"
#include "OpenGL.h"

bool OpenGL::Initialize() {

	EnableOpenGLDebugOutput();

	Shader lightingShader = Shader();
	Shader screenShader = Shader();
	Shader gBufferShader = Shader();
	Shader shadowMapShader = Shader();
	Shader pointShadowMapShader = Shader();

	lightingShader.Compile("Lighting", "Assets/Shaders/lighting.vert", "Assets/Shaders/lighting.frag");
	screenShader.Compile("Screen", "Assets/Shaders/screen.vert", "Assets/Shaders/screen.frag");
	gBufferShader.Compile("gBuffer", "Assets/Shaders/gbuffer.vert", "Assets/Shaders/gbuffer.frag");
	shadowMapShader.Compile("ShadowMap", "Assets/Shaders/shadow_map.vert", "Assets/Shaders/shadow_map.frag");
	pointShadowMapShader.Compile("PointShadowMap", "Assets/Shaders/point_shadow_map.vert", "Assets/Shaders/point_shadow_map.frag", "Assets/Shaders/point_shadow_map.geom");

	mCompiledShaders.emplace(SHADER_LIGHTING, lightingShader);
	mCompiledShaders.emplace(SHADER_SCREEN, screenShader);
	mCompiledShaders.emplace(SHADER_GBUFFER, gBufferShader);
	mCompiledShaders.emplace(SHADER_SHADOW_MAP, shadowMapShader);
	mCompiledShaders.emplace(SHADER_POINT_SHADOW_MAP, pointShadowMapShader);

	pointLightDataArraySSBO = ShaderStorageBuffer();
	pointLightDataArraySSBO.Generate("pointLightDataArray", SSBO_POINT_LIGHT_DATA_ARRAY);
	pointLightDataArraySSBO.Bind();
	pointLightDataArraySSBO.Allocate(MAX_POINT_LIGHTS * sizeof(PointLightData));
	pointLightDataArraySSBO.Unbind();

	cameraDataUBO = UniformBuffer();
	cameraDataUBO.Generate("cameraData", CAMERA_UNIFORM_BUFFER_INDEX, sizeof(CameraData));
	cameraDataUBO.Bind();
	cameraDataUBO.Allocate(sizeof(CameraData));
	cameraDataUBO.Unbind();

	// a single VAO for pointing to a VBO and EBO
	// dedicated to storing mesh data
	meshVAO = VertexArrayBuffer();
	meshVAO.Generate("meshes");
	meshVAO.Bind();

	meshVBO = VertexBuffer();
	meshVBO.Generate("meshes");
	meshVBO.Bind();
	meshVBO.Allocate(MAX_MESHES * sizeof(Vertex));

	meshEBO = ElementArrayBuffer();
	meshEBO.Generate("meshes");
	meshEBO.Bind();
	meshEBO.Allocate(MAX_MESHES * sizeof(GLint));

	meshVAO.SetAttribPointer(0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	meshVAO.SetAttribPointer(1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	meshVAO.SetAttribPointer(2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	meshVAO.Unbind();
	meshEBO.Unbind();
	meshVBO.Unbind();

	// storing mesh instance data
	// we initially allocate data for
	// MAX_MESHES, where each mesh
	// allows for MAX_MESH_INSTANCES of
	// the MeshInstanceData struct
	meshSSBO = ShaderStorageBuffer();
	meshSSBO.Generate("mesh_instances", SSBO_MESH_INSTANCE_DATA);
	meshSSBO.Bind();
	meshSSBO.Allocate(MAX_MESH_INSTANCES * sizeof(MeshInstanceData));
	meshSSBO.Unbind();	

	materialSSBO = ShaderStorageBuffer();
	materialSSBO.Generate("materials", SSBO_MATERIAL_INSTANCE_DATA);
	materialSSBO.Bind();
	materialSSBO.Allocate(MAX_MESHES * MAX_MATERIAL_INSTANCES * sizeof(MaterialData));
	materialSSBO.Unbind();

	// initialize the draw indirect 
	// buffer object to store draw 
	// commands. 
	// we initially allocate it to
	// MAX_MESHES since there should
	// be one draw call command for
	// each one of the meshes.
	meshDIB = DrawIndirectBuffer();
	meshDIB.Generate("mesh_instances");
	meshDIB.Bind();
	meshDIB.Allocate(MAX_MESHES * sizeof(DrawIndirectElementCommand));
	meshDIB.Unbind();

	InitShadowMap();
	InitGBuffer();
	BufferScreenQuad();

	glEnable(GL_CULL_FACE);

	// all good
	return true;
}

void OpenGL::InitShadowMap() {
	// directional and spot lights
	shadowMapFBO = FrameBuffer();
	shadowMapFBO.Generate("shadowMap", SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	shadowMapFBO.Bind();
	shadowMapFBO.CreateDepthAttachment(GL_DEPTH_COMPONENT32F);
	shadowMapFBO.DisableColorBuffer();

	if (!shadowMapFBO.CheckComplete()) {
		std::cout << "OpenGL: Shadow map frame buffer is not complete!" << std::endl;
	}
		
	shadowMapFBO.Unbind();

	// point shadows
	pointShadowCubemapArray = TextureCubeMapArray();
	pointShadowCubemapArray.Generate(GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 6);

	pointShadowFBO = FrameBuffer();
	pointShadowFBO.Generate("pointShadowFBO", SCR_WIDTH, SCR_HEIGHT);
	pointShadowFBO.Bind();
	pointShadowFBO.AttachCubeMapTexture(pointShadowCubemapArray.name, pointShadowCubemapArray.id);
	pointShadowFBO.DisableColorBuffer();
	pointShadowFBO.Unbind();

	if (!pointShadowFBO.CheckComplete()) {
		throw std::runtime_error("OpenGL: Point shadow map frame buffer is not complete!");
	}
}

void OpenGL::InitGBuffer() {
	gPosition = Texture2D();
	gPosition.Generate("gPosition", nullptr, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA16F, GL_RGBA, 0, GL_FLOAT);
	gPosition.Bind();
	gPosition.SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gPosition.SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gNormal = Texture2D();
	gNormal.Generate("gNormal", nullptr, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA16F, GL_RGBA, 0, GL_FLOAT);
	gNormal.Bind();
	gNormal.SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gNormal.SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gAlbedoSpec = Texture2D();
	gAlbedoSpec.Generate("gAlbedoSpec", nullptr, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA8, GL_RGBA, 0, GL_FLOAT);
	gAlbedoSpec.Bind();
	gAlbedoSpec.SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gAlbedoSpec.SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gAlbedoSpec.Unbind();

	gBuffer = FrameBuffer();
	gBuffer.Generate("gBuffer", SCR_WIDTH, SCR_HEIGHT);
	gBuffer.Bind();
	gBuffer.AttachColorTexture2D(gPosition.name, gPosition.id, 0);
	gBuffer.AttachColorTexture2D(gNormal.name, gNormal.id, 1);
	gBuffer.AttachColorTexture2D(gAlbedoSpec.name, gAlbedoSpec.id, 2);
	gBuffer.DrawColorBuffers();
	gBuffer.CreateRenderBuffer();
	gBuffer.Unbind();

	if (!gBuffer.CheckComplete()) {
		throw std::runtime_error("OpenGL: G-buffer frame buffer is not complete!");
	}
}

void OpenGL::BufferTexture(Texture* texture) {
	//assert(texture != nullptr && "OpenGL: Failed to buffer texture.");

	//if (assetToTextureIDMap.count(texture->assetId)) { // texture already loaded
	//	return;
	//}

	//GLenum format = GL_RED;
	//GLenum internalFormat = GL_R8;
	//if (texture->nrChannels == 3)
	//{
	//	format = GL_RGB;
	//	internalFormat = GL_RGB8;
	//}
	//else if (texture->nrChannels == 4)
	//{
	//	format = GL_RGBA;
	//	internalFormat = GL_RGBA8;
	//}

	//Texture2D glTexture = Texture2D();
	//glTexture.Generate(texture->assetName, texture->data, 0, texture->width, texture->height, internalFormat, format, 0, GL_UNSIGNED_BYTE);
	//glTexture.Bind();
	//glTexture.SetInt(GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexture.SetInt(GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexture.SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexture.SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexture.GenerateMipMap();
	//glTexture.Unbind();

	//assetToTextureIDMap.emplace(texture->assetId, glTexture.id);
}

void OpenGL::BufferScreenQuad() {
	screenQuadVAO = VertexArrayBuffer();
	screenQuadVAO.Generate("screenQuad");
	screenQuadVBO = VertexBuffer();
	screenQuadVBO.Generate("screenQuad");

	screenQuadVAO.Bind();
	screenQuadVBO.Bind();

	screenQuadVBO.BufferData(sizeof(quadVertices), quadVertices);

	screenQuadVAO.SetAttribPointer(0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
	screenQuadVAO.SetAttribPointer(1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	screenQuadVAO.Unbind();
	screenQuadVBO.Unbind();
}

glm::mat4 OpenGL::CalculateModelMatrix(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale)
{
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = modelMatrix * glm::mat4_cast(rotation);
	modelMatrix = glm::scale(modelMatrix, scale);
	return modelMatrix;
}

void OpenGL::UploadSceneLightData() {
	SceneLightData sceneLightData;
	sceneLightData.mHasDirectionalLight = directionalLight != nullptr;
	sceneLightData.mDirectionalLightColor = directionalLight? directionalLight->color : glm::vec3(1.0f);
	sceneLightData.mDirectionalLightDirection = directionalLight? directionalLight->direction : glm::vec3(1.0f);
	sceneLightData.mDirectionalLightIntensity = directionalLight? directionalLight->intensity : 1.0f;
	sceneLightData.mDirectionalLightMatrix = directionalLight? directionalLight->LightSpaceMatrix(currentActiveCamera->position) : glm::mat4(1.0f);
	sceneLightData.mAmbientLightColor = glm::vec3(1.0f);
	sceneLightData.mAmbientLightIntensity = 0.1f;
	sceneLightData.mPointLightsCount = static_cast<GLsizei>(pointLights.size());

	sceneLightDataUBO.Bind();
	sceneLightDataUBO.BufferSubData(0, sizeof(SceneLightData), &sceneLightData);
	sceneLightDataUBO.Unbind();

	PointLightsDataArray pointLightsDataArray;
	for (int i = 0; i < pointLights.size(); i++) {
		PointLight* light = pointLights[i];
		if (light == nullptr) {
			continue;
		}

		PointLightData pointLightData;
		pointLightData.mColor = light->color;
		pointLightData.mIntensity = light->intensity;
		pointLightData.mPosition = light->position;
		pointLightData.mRadius = light->radius;
		pointLightData.shadowFarPlane = light->shadowMapFarPlane;
		pointLightData.shadowCubeMapIndex = light->glShadowMapIndex;

		pointLightsDataArray.mPointLights[i] = pointLightData;
	}

	pointLightDataArraySSBO.Bind();
	// TODO fix me to use offsets correctly
	//pointLightDataArraySSBO.BufferSubData(pointLights.size() * sizeof(PointLightData), &pointLightsDataArray.mPointLights);
	pointLightDataArraySSBO.Unbind();
}

void OpenGL::UploadCameraData() {
	assert(currentActiveCamera && "OpenGL: Attempting to upload camera data without an active camera.");

	CameraData cameraData = {};
	cameraData.mPosition = glm::vec4(currentActiveCamera->position, 1.0f);
	cameraData.mProjection = currentActiveCamera->ProjectionMatrix();
	cameraData.mView = currentActiveCamera->ViewMatrix();

	cameraDataUBO.Bind();
	cameraDataUBO.BufferSubData(0, sizeof(CameraData), &cameraData);
	cameraDataUBO.Unbind();
}

void OpenGL::PointShadowMapPass() {
	if (pointLights.empty()) {
		return;
	}

	pointShadowFBO.Bind();
	pointShadowFBO.SetViewport();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_ARRAY);
	glCullFace(GL_FRONT);

	Shader pointShadowShader = mCompiledShaders[SHADER_POINT_SHADOW_MAP];
	pointShadowShader.Bind();

	size_t pointLightsCount = pointLights.size();
	pointShadowCubemapArray.Bind();
	pointShadowCubemapArray.Allocate(GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 6 * pointLightsCount);

	meshSSBO.Bind();
	meshDIB.Bind();
	meshVAO.Bind();

	for (unsigned int i = 0; i < pointLightsCount; i++) {
		glClear(GL_DEPTH_BUFFER_BIT);
		PointLight* pointLight = pointLights[i];
		pointLight->glShadowMapIndex = i;

		std::vector<glm::mat4> cubemapViews = pointLight->GetCubemapLightSpaceMatrices(currentActiveCamera->position);
		pointShadowShader.SetUMat4v("uCubeMapMatrices", cubemapViews.size(), cubemapViews.data());

		pointShadowCubemapArray.SetLevelParameters(i);

		meshDIB.Draw(meshDrawCommandDataArray.size()); // draw scene (we assume that mesh data has already been buffered in the geometry pass
	}

	glCullFace(GL_BACK);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	meshVAO.Unbind();
	meshDIB.Unbind();
	meshSSBO.Unbind();
	pointShadowCubemapArray.Unbind();
	pointShadowFBO.Unbind();
}

void OpenGL::ShadowMapPass() {
	if (directionalLight == nullptr) {
		return;
	}

	shadowMapFBO.SetViewport();
	shadowMapFBO.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	Shader shadowShader = mCompiledShaders[SHADER_SHADOW_MAP];
	shadowShader.Bind();
	shadowShader.SetUMat4("uLightSpaceMatrix", directionalLight->LightSpaceMatrix(currentActiveCamera->position));

	meshSSBO.Bind();
	meshDIB.Bind();
	meshVAO.Bind();

	//meshSSBO.BufferData(meshInstanceDataArray.size() * sizeof(MeshInstanceData), meshInstanceDataArray.data());
	//meshDIB.BufferData(meshDrawCommandArray.size() * sizeof(DrawIndirectElementCommand), meshDrawCommandArray.data());

	meshDIB.Draw(meshDrawCommandDataArray.size());

	meshVAO.Unbind();
	meshDIB.Unbind();
	meshSSBO.Unbind();
	shadowMapFBO.Unbind();
	
	glCullFace(GL_BACK);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void OpenGL::LightingPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader lightingShader = mCompiledShaders[SHADER_LIGHTING];
	lightingShader.Bind();

	UploadSceneLightData();

	pointShadowCubemapArray.ActiveAndBind(0);
	directionalLightShadowMap.ActiveAndBind(1);
	gPosition.ActiveAndBind(2);
	gNormal.ActiveAndBind(3);
	gAlbedoSpec.ActiveAndBind(4);

	glDisable(GL_DEPTH_TEST);
	screenQuadVAO.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	screenQuadVAO.Unbind();

	glEnable(GL_DEPTH_TEST);
	pointShadowCubemapArray.Unbind();
	directionalLightShadowMap.Unbind();
	gPosition.Unbind();
	gNormal.Unbind();
	gAlbedoSpec.Unbind();
}

void OpenGL::DebugGbuffer(uint32_t layer) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader shaderScreen = mCompiledShaders[SHADER_SCREEN];
	shaderScreen.Bind();

	if (layer == 0) {
		gPosition.ActiveAndBind(0);
	}
	if (layer == 1) {
		gNormal.ActiveAndBind(0);
	}
	if (layer == 2) {
		gAlbedoSpec.ActiveAndBind(0);
	}
	if (layer == 3) {
		directionalLightShadowMap.ActiveAndBind(0);
	}

	glDisable(GL_DEPTH_TEST);
	screenQuadVAO.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	screenQuadVAO.Unbind();
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}

void OpenGL::RegisterStaticMeshInstance(const AssetId assetID, Entity entity, const Transform* transform) {
	if (transform == nullptr)
		return;

	MeshData& meshData = meshDataMap[assetID];
	DrawIndirectElementCommand& command = meshDrawCommandDataArray[meshData.drawCommandId];
	std::vector<MeshInstanceData>& meshInstances = meshInstanceMap[assetID];

	// check if instance is already registered
	MeshInstanceData* instanceData = nullptr;
	auto it = entityToInstanceIDMap.find(entity);
	if (it != entityToInstanceIDMap.end()) {
		instanceData = &meshInstances[it->second];
	}
	else {
		entityToInstanceIDMap[entity] = meshInstances.size();

		meshInstances.emplace_back(MeshInstanceData());
		instanceData = &meshInstances.back();
		command.instanceCount++;
	}

	instanceData->model = CalculateModelMatrix(transform->position, transform->rotation, transform->scale);
	instanceData->inverseModel = glm::inverseTranspose(instanceData->model);
}

void OpenGL::RegisterStaticMesh(const StaticMesh* sm) {
	if (sm == nullptr)
		return;

	AssetId assetID = sm->assetId;

	// check if mesh is already buffered to the GPU
	if (meshDataMap.find(assetID) == meshDataMap.end()) {

		const std::vector<Vertex>& meshVertices = sm->vertices;
		const std::vector<GLuint>& meshIndices = sm->indices;

		MeshData meshData;
		meshData.indexCount = meshIndices.size();
		meshData.firstIndex = meshIndicesDataArray.size();
		meshData.baseVertex = meshVertexDataArray.size();
		meshData.drawCommandId = meshDrawCommandDataArray.size();
		meshDataMap[assetID] = meshData;

		meshIndicesDataArray.insert(meshIndicesDataArray.end(), meshIndices.begin(), meshIndices.end());
		meshVertexDataArray.insert(meshVertexDataArray.end(), meshVertices.begin(), meshVertices.end());

		DrawIndirectElementCommand command;
		command.count = meshData.indexCount;
		command.instanceCount = 0;
		command.firstIndex = meshData.firstIndex;
		command.baseVertex = meshData.baseVertex;
		
		meshDrawCommandDataArray.push_back(command);
	}
}

void OpenGL::BatchUploadStaticMeshData() {
	meshVBO.Bind();
	meshEBO.Bind();

	meshVBO.BufferData(meshVertexDataArray.size() * sizeof(Vertex), meshVertexDataArray.data());
	meshEBO.BufferData(meshIndicesDataArray.size() * sizeof(GLuint), meshIndicesDataArray.data());

	meshVBO.Unbind();
	meshEBO.Unbind();
}

void OpenGL::BatchUploadStaticMeshInstanceData() {
	std::vector<MeshInstanceData> meshInstanceDataArray;
	for (auto& it : meshInstanceMap) {
		const MeshData& meshData = meshDataMap[it.first];

		std::vector<MeshInstanceData>& instancesData = it.second;
		DrawIndirectElementCommand& command = meshDrawCommandDataArray[meshData.drawCommandId];

		command.baseInstance = meshInstanceDataArray.size();
		meshInstanceDataArray.insert(meshInstanceDataArray.end(), instancesData.begin(), instancesData.end());
	}

	meshDIB.Bind();
	meshSSBO.Bind();
	
	meshDIB.BufferData(meshDrawCommandDataArray.size() * sizeof(DrawIndirectElementCommand), meshDrawCommandDataArray.data());
	meshSSBO.BufferData(meshInstanceDataArray.size() * sizeof(MeshInstanceData), meshInstanceDataArray.data());
	
	meshDIB.Unbind();
	meshSSBO.Unbind();
}

void OpenGL::GeometryPass() {
	Shader gBufferShader = mCompiledShaders[SHADER_GBUFFER];
	gBufferShader.Bind();

	gBuffer.Bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	meshSSBO.Bind();
	meshDIB.Bind();
	meshVAO.Bind();

	meshDIB.Draw(meshDrawCommandDataArray.size());

	gBuffer.Unbind();
	meshVAO.Unbind();
	meshDIB.Unbind();
	meshSSBO.Unbind();
}
