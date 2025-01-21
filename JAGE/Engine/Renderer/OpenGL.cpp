#include <ECS/EntityManager.h>
#include <Core/Types/GameAsset.h>
#include <Core/Types/Texture.h>
#include <Core/Types/Mesh.h>
#include <Core/Types/Material.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Components/PointLight.h>
#include <ECS/Components/StaticMeshRenderer.h>
#include <Utils.h>
#include <LogDisplay.h>

#include "OpenGLUtils.h"
#include "OpenGL.h"

bool OpenGL::Initialize() {

	EnableOpenGLDebugOutput();

	lightingShader = Shader("Assets/Shaders/lighting.vert", "Assets/Shaders/lighting.frag");
	screenShader = Shader("Assets/Shaders/screen.vert", "Assets/Shaders/screen.frag");
	gBufferShader = Shader("Assets/Shaders/gbuffer.vert", "Assets/Shaders/gbuffer.frag");
	shadowMapShader = Shader("Assets/Shaders/shadow_map.vert", "Assets/Shaders/shadow_map.frag");
	pointShadowMapShader = Shader("Assets/Shaders/point_shadow_map.vert", "Assets/Shaders/point_shadow_map.frag", "Assets/Shaders/point_shadow_map.geom");

	meshVBO = VertexArrayBuffer(5000 * MAX_MESHES, GL_DYNAMIC_STORAGE_BIT);
	meshEBO = ElementArrayBuffer(3 * 5000 * MAX_MESHES, GL_DYNAMIC_STORAGE_BIT);
	meshDIB = DrawIndirectBuffer(SSBO_MESH_INDIRECT_DRAW_COMMAND, MAX_MESHES * sizeof(MeshDrawCmdData), GL_DYNAMIC_STORAGE_BIT);

	meshSSBO = ShaderStorageBuffer(SSBO_MESH_INSTANCE_DATA, MAX_MESHES * sizeof(MeshInstanceData));
	textureHndlrsSSBO = ShaderStorageBuffer(SSBO_TEXTURE_HANDLERS, MAX_TEXTURES * sizeof(GLuint64));
	pointLightDataArraySSBO = ShaderStorageBuffer(SSBO_POINT_LIGHT_DATA_ARRAY, MAX_POINT_LIGHTS * sizeof(PointLightData));
	
	cameraDataUBO = UniformBuffer(UBO_CAMERA_DATA, sizeof(CameraData));
	sceneLightDataUBO = UniformBuffer(UBO_SCENE_LIGHT_DATA, sizeof(SceneLightData), GL_DYNAMIC_STORAGE_BIT);

	meshVAO = VertexArray();
	std::vector<VertexAttrib> meshVAOAttribs = {
		{ 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position) },
		{ 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal) },
		{ 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords) }
	};
	meshVAO.Configure(meshVBO.GetID(), sizeof(Vertex), meshEBO.GetID(), meshVAOAttribs);

	
	screenQuadVBO = VertexArrayBuffer(sizeof(quadVertices), GL_DYNAMIC_STORAGE_BIT);
	screenQuadVBO.UpdateData(0, sizeof(quadVertices), quadVertices);
	screenQuadEBO = ElementArrayBuffer(sizeof(quadIndices), GL_DYNAMIC_STORAGE_BIT);
	screenQuadEBO.UpdateData(0, sizeof(quadIndices), quadIndices);

	screenQuadVAO = VertexArray();
	std::vector<VertexAttrib> screenVAOAttribs = {
		{0, 2, GL_FLOAT, GL_FALSE, 0},
		{1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)}
	};
	screenQuadVAO.Configure(screenQuadVBO.GetID(), 4 * sizeof(float), screenQuadEBO.GetID(), screenVAOAttribs);

	InitShadowMap();
	InitGBuffer();

	glEnable(GL_CULL_FACE);

	return true;
}

void OpenGL::InitShadowMap() {
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

	// directional and spot lights
	shadowMapFBO = FrameBuffer(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	directionalLightShadowMap = Texture2D("dir_shadow_map", GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	directionalLightShadowMap.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	directionalLightShadowMap.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	directionalLightShadowMap.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	directionalLightShadowMap.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	directionalLightShadowMap.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);

	shadowMapFBO.AttachDepthTex2D(directionalLightShadowMap.GetID());
	shadowMapFBO.DisableColorBuffer();

	if (!shadowMapFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Shadow map frame buffer is incomplete.");
		return;
	}
		
	// point shadow map
	BIND(Shader, pointShadowMapShader); // we bind here to tell open gl that we are using a geometry shader
	pointShadowFBO = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	pointShadowCubemapArray = TextureCubeMapArray("point_shadow_cubemap_array", GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 6 * MAX_POINT_LIGHTS, 1);
	pointShadowCubemapArray.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);
	pointShadowFBO.AttachDepthCubeMapTex(pointShadowCubemapArray.GetID());
	pointShadowFBO.DisableColorBuffer();

	if (!pointShadowFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Point shadow map frame buffer is incomplete.");
		return;
	}
}

void OpenGL::InitGBuffer() {
	gPosition = Texture2D("gPostition", GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT);
	gPosition.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gPosition.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gNormal = Texture2D("gNormal", GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT);
	gNormal.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gNormal.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gAlbedoSpec = Texture2D("gAlbedoSpec", GL_RGBA8, SCR_WIDTH, SCR_HEIGHT);
	gAlbedoSpec.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gAlbedoSpec.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gBuffer = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	gBuffer.AttachColorTex2D(gPosition.GetID(), 0);
	gBuffer.AttachColorTex2D(gNormal.GetID(), 1);
	gBuffer.AttachColorTex2D(gAlbedoSpec.GetID(), 2);
	gBuffer.DrawColorBuffers();
	gBuffer.CreateRenderBuffer();

	if (!gBuffer.CheckComplete()) {
		throw std::runtime_error("OpenGL: G-buffer frame buffer is not complete!");
	}
}

void OpenGL::RegisterTexture2D(Texture* texture) {
	assert(texture != nullptr && "OpenGL: Failed to buffer texture.");

	auto it = assetIDToTex2DMap.find(texture->assetId);
	if (it != assetIDToTex2DMap.end())
		return;

	GLenum format = GL_RED;
	GLenum internalFormat = GL_R8;
	if (texture->nrChannels == 3)
	{
		format = GL_RGB;
		internalFormat = GL_RGB8;
	}
	else if (texture->nrChannels == 4)
	{
		format = GL_RGBA;
		internalFormat = GL_RGBA8;
	}

	Texture2D glTex2D = Texture2D(texture->assetName, internalFormat, texture->width, texture->height);
	glTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTex2D.GenerateMipMap();
	glTex2D.SetSubImage2D(format, 0, 0, 0, GL_UNSIGNED_BYTE, texture->data);
	glTex2D.MakeResident();
	glTex2D.SetHandleIndex(tex2DHndlrDataArray.size());

	GLuint64 hndl = glTex2D.GetHandle();
	textureHndlrsSSBO.UpdateData(tex2DHndlrDataArray.size() * sizeof(GLuint64), sizeof(GLuint64), &hndl);
	tex2DHndlrDataArray.push_back(glTex2D.GetHandle());

	assetIDToTex2DMap[texture->assetId] = glTex2D;
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
	sceneLightData.mHasDirectionalLight = static_cast<GLuint>(directionalLight != nullptr);
	sceneLightData.mDirectionalLightColor = directionalLight? directionalLight->color : glm::vec3(1.0f);
	sceneLightData.mDirectionalLightDirection = directionalLight? directionalLight->direction : glm::vec3(1.0f);
	sceneLightData.mDirectionalLightIntensity = directionalLight? directionalLight->intensity : 1.0f;
	sceneLightData.mDirectionalLightMatrix = directionalLight? directionalLight->LightSpaceMatrix(currentActiveCamera->position) : glm::mat4(1.0f);
	sceneLightData.mAmbientLightColor = glm::vec3(1.0f);
	sceneLightData.mAmbientLightIntensity = .1f;
	sceneLightData.mPointLightsCount = static_cast<GLsizei>(pointLightDataArray.size());

	sceneLightDataUBO.UpdateData(0, sizeof(SceneLightData), &sceneLightData);
}

void OpenGL::UploadCameraData() {
	assert(currentActiveCamera && "OpenGL: Attempting to upload camera data without an active camera.");

	CameraData cameraData = {};
	cameraData.mPosition = glm::vec4(currentActiveCamera->position, 1.0f);
	cameraData.mProjection = currentActiveCamera->ProjectionMatrix();
	cameraData.mView = currentActiveCamera->ViewMatrix();

	cameraDataUBO.UpdateData(0, sizeof(CameraData), &cameraData);
}

void OpenGL::PointShadowMapPass() {
	if (pointLightDataArray.empty())
		return;

	BIND(FrameBuffer, pointShadowFBO);
	pointShadowFBO.SetViewport();

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	size_t pointLightsCount = pointLightDataArray.size();

	BIND(Shader, pointShadowMapShader);
	BIND(VertexArray, meshVAO);
	BIND(DrawIndirectBuffer, meshDIB);

	// we re-draw the scene for each of the faces
	// of the cube map depth texture
	for (unsigned int i = 0; i < pointLightsCount; i++) {
		
		PointLightData& lightData = pointLightDataArray[i];
		lightData.shadowCubeMapIndex = i;
		
		glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, lightData.shadowNearPlane, lightData.shadowFarPlane);

		// generate 6 shadow maps, for each cube face
		std::vector <glm::mat4> views;
		glm::vec3 cameraPos = currentActiveCamera->position;
		views.push_back(projection *
			glm::lookAt(lightData.mPosition, lightData.mPosition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		views.push_back(projection *
			glm::lookAt(lightData.mPosition, lightData.mPosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		views.push_back(projection *
			glm::lookAt(lightData.mPosition, lightData.mPosition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		views.push_back(projection *
			glm::lookAt(lightData.mPosition, lightData.mPosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		views.push_back(projection *
			glm::lookAt(lightData.mPosition, lightData.mPosition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		views.push_back(projection *
			glm::lookAt(lightData.mPosition, lightData.mPosition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

		pointShadowMapShader.SetUMat4v("uCubeMapMatrices", views.size(), views.data());
		pointShadowMapShader.SetUVec3("uLightPos", lightData.mPosition);
		pointShadowMapShader.SetUFloat("uLightFarPlane", lightData.shadowFarPlane);
		pointShadowMapShader.SetUInt("uBaseLayerOffset", 6*i);

		pointLightDataArraySSBO.UpdateData(lightData.dataArrayIndex * sizeof(PointLightData) + offsetof(PointLightData, shadowCubeMapIndex), sizeof(GLuint), &lightData.shadowCubeMapIndex);

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, meshDrawCmdDataArray.size(), sizeof(MeshDrawCmdData));
	}

	glCullFace(GL_BACK);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void OpenGL::ShadowMapPass() {
	if (directionalLight == nullptr) {
		return;
	}

	BIND(Shader, shadowMapShader);
	BIND(FrameBuffer, shadowMapFBO);
	BIND(DrawIndirectBuffer, meshDIB);
	BIND(VertexArray, meshVAO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	shadowMapFBO.SetViewport();
	shadowMapShader.SetUMat4("uLightSpaceMatrix", directionalLight->LightSpaceMatrix(currentActiveCamera->position));
	
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, meshDrawCmdDataArray.size(), sizeof(MeshDrawCmdData));
	
	glCullFace(GL_BACK);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void OpenGL::LightingPass() {
	BIND(Shader, lightingShader);
	BIND(VertexArray, screenQuadVAO);
	BIND_TEX(TextureCubeMapArray, pointShadowCubemapArray, 0);
	BIND_TEX(Texture2D, directionalLightShadowMap, 1);
	BIND_TEX(Texture2D, gPosition, 2);
	BIND_TEX(Texture2D, gNormal, 3);
	BIND_TEX(Texture2D, gAlbedoSpec, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
}

void OpenGL::DebugGbuffer(uint32_t layer) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BIND(Shader, screenShader);
	BIND(VertexArray, screenQuadVAO);

	Texture2D& textToUse = gPosition;
	if (layer == 0) {
		// keep gPosiiton as default
	}
	else if (layer == 1) {
		textToUse = gNormal;
	}
	else if (layer == 2) {
		textToUse = gAlbedoSpec;
	}
	else if (layer == 3) {
		textToUse = directionalLightShadowMap;
	}
	else {
		LOG(LogOpenGL, LOG_WARNING, "Debug mode for gBuffer failed. Invalid layer.");
	}

	BIND_TEX(Texture2D, textToUse, 0);

	glDisable(GL_DEPTH_TEST);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glEnable(GL_DEPTH_TEST);
}

void OpenGL::UpsertMeshEntity(const Entity entity, const std::vector<const Mesh*>& meshes, const Transform& transform) {
	glm::mat4 model = CalculateModelMatrix(transform.position, transform.rotation, transform.scale);;
	glm::mat4 inverseModel = glm::inverseTranspose(model);

	for (const Mesh* mesh : meshes) {
		auto meshDataIt = assToMesh.find(mesh->assetId);
		if (assToMesh.find(mesh->assetId) == assToMesh.end()) {
			LOG(LogOpenGL, LOG_ERROR, std::format("Missing open GL mesh data for mesh {}.", mesh->assetPath));
			continue;
		}

		auto& meshInstances = assToMeshInsts[mesh->assetId];
		auto [entInstIt, inserted] = assToEntMeshInstIndexes[mesh->assetId].try_emplace(entity, meshInstances.size());

		if (inserted) {
			meshInstances.emplace_back();
			meshDrawCmdDataArray[meshDataIt->second.drawCmdIndex].instanceCount++;
		}

		auto& instance = meshInstances[entInstIt->second];
		instance.model = model;
		instance.inverseModel = inverseModel;
	}	
}

void OpenGL::RegisterPointLight(const Entity entity, const PointLight* light) {
	if (light == nullptr) {
		LOG(LogOpenGL, LOG_WARNING, "Failed to register point light. Light is null.");
		return;
	}

	auto [it, inserted] = entToPointLightData.try_emplace(entity);

	it->second.mColor = light->color;
	it->second.mIntensity = light->intensity;
	it->second.mPosition = light->position;
	it->second.mRadius = light->radius;
	it->second.shadowFarPlane = light->shadowMapFarPlane;

	if (inserted) {
		it->second.dataArrayIndex = pointLightDataArray.size();
		pointLightDataArray.push_back(it->second);
	}

	pointLightDataArraySSBO.UpdateData(it->second.dataArrayIndex * sizeof(PointLightData), sizeof(PointLightData), &it->second);
	// pointShadowCubemapArray.Allocate(GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 6 ^ pointLightDataArray.size());
}

void OpenGL::RegisterMesh(const Mesh* mesh) {
	const std::vector<Vertex>& vertices = mesh->vertices;
	const std::vector<GLuint>& indices = mesh->indices;

	auto [it, inserted] = assToMesh.try_emplace(mesh->assetId);
	if (inserted) {
		it->second.drawCmdIndex = meshDrawCmdDataArray.size();

		meshDrawCmdDataArray.push_back(MeshDrawCmdData());
		MeshDrawCmdData& cmd = meshDrawCmdDataArray.back();

		cmd.count = indices.size();
		cmd.instanceCount = 0;
		cmd.firstIndex = meshIndexDataArray.size();
		cmd.baseVertex = meshVertexDataArray.size();

		cmd.diffTexHndlrIndex = mesh->diffuseTexture >= 0 ? assetIDToTex2DMap[mesh->diffuseTexture].GetHandleIndex() : -1;
		cmd.specTexHndlrIndex = mesh->specularTexture >= 0 ? assetIDToTex2DMap[mesh->specularTexture].GetHandleIndex() : -1;
		cmd.normTexHndlrIndex = mesh->normalTexture >= 0 ? assetIDToTex2DMap[mesh->normalTexture].GetHandleIndex() : -1;
		
		meshIndexDataArray.insert(meshIndexDataArray.end(), indices.begin(), indices.end());
		meshVertexDataArray.insert(meshVertexDataArray.end(), vertices.begin(), vertices.end());

		meshVBO.UpdateData(cmd.baseVertex * sizeof(Vertex), vertices.size() * sizeof(Vertex), vertices.data());
		meshEBO.UpdateData(cmd.firstIndex * sizeof(GLuint), indices.size() * sizeof(GLuint), indices.data());
	}
	else {
		LOG(LogOpenGL, LOG_WARNING, std::format("Mesh with asset id {} already registered. Ignoring...", mesh->assetId));
	}
}

void OpenGL::BatchMeshInstData() {
	std::vector<MeshInstanceData> meshInstDataArray;
	for (const auto& it : assToMeshInsts) {
		MeshMetaData& metaData = assToMesh[it.first];
		MeshDrawCmdData& cmdData = meshDrawCmdDataArray[metaData.drawCmdIndex];
		cmdData.baseInstance = meshInstDataArray.size();

		const std::vector<MeshInstanceData>& meshInsts = it.second;
		meshInstDataArray.insert(meshInstDataArray.end(), meshInsts.begin(), meshInsts.end());
	}

	meshDIB.UpdateData(0, meshDrawCmdDataArray.size() * sizeof(MeshDrawCmdData), meshDrawCmdDataArray.data());
	meshSSBO.UpdateData(0, meshInstDataArray.size() * sizeof(MeshInstanceData), meshInstDataArray.data());
}

void OpenGL::GeometryPass() {
	BIND(Shader, gBufferShader);
	BIND(FrameBuffer, gBuffer);
	BIND(DrawIndirectBuffer, meshDIB);
	BIND(VertexArray, meshVAO);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, meshDrawCmdDataArray.size(), sizeof(MeshDrawCmdData));
}
