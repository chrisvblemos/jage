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
#include <Core/AssetManager.h>
#include <Utils.h>
#include <LogDisplay.h>

#include "ShaderPreProcessor.h"
#include "OpenGLUtils.h"
#include "OpenGL.h"

bool OpenGL::Initialize() {

	EnableOpenGLDebugOutput();

	ShaderPreProcessor spp = ShaderPreProcessor();
	spp.Initialize();
	
	lightingShader = Shader(spp.GetCodeStr("lighting.vert"), spp.GetCodeStr("lighting.frag"));
	screenShader = Shader(spp.GetCodeStr("screen.vert"), spp.GetCodeStr("screen.frag"));
	gBufferShader = Shader(spp.GetCodeStr("gbuffer.vert"), spp.GetCodeStr("gbuffer.frag"));
	pointLightShadowMapShader = Shader(spp.GetCodeStr("point_shadow_map.vert"), spp.GetCodeStr("point_shadow_map.frag"), spp.GetCodeStr("point_shadow_map.geom"));
	varianceShadowMapShader = Shader(spp.GetCodeStr("vsm.vert"), spp.GetCodeStr("vsm.frag"));
	hBlurShadowMapShader = Shader(spp.GetCodeStr("screen.vert"), spp.GetCodeStr("gaussian_blur_h.frag"));
	vBlurShadowMapShader = Shader(spp.GetCodeStr("screen.vert"), spp.GetCodeStr("gaussian_blur_v.frag"));
	shadowMapShader = Shader(spp.GetCodeStr("csm.vert"), spp.GetCodeStr("csm.frag"), spp.GetCodeStr("csm.geom"));
	ssaoShader = Shader(spp.GetCodeStr("ssao.vert"), spp.GetCodeStr("ssao.frag"));
	ssaoBlurShader = Shader(spp.GetCodeStr("ssao.vert"), spp.GetCodeStr("ssao_blur.frag"));
	postFxShader = Shader(spp.GetCodeStr("screen.vert"), spp.GetCodeStr("post_fx.frag"));

	meshVBO = VertexArrayBuffer(5000 * MAX_MESHES, GL_DYNAMIC_STORAGE_BIT);
	meshEBO = ElementArrayBuffer(3 * 5000 * MAX_MESHES, GL_DYNAMIC_STORAGE_BIT);
	meshDIB = DrawIndirectBuffer(SSBO_MESH_INDIRECT_DRAW_COMMAND, MAX_MESHES * sizeof(MeshDrawCmdData), GL_DYNAMIC_STORAGE_BIT);

	meshSSBO = ShaderStorageBuffer(SSBO_MESH_INSTANCE_DATA, MAX_MESHES * sizeof(MeshInstanceData));
	textureHndlrsSSBO = ShaderStorageBuffer(SSBO_TEXTURE_HANDLERS, MAX_TEXTURES * sizeof(GLuint64));
	pointLightDataArraySSBO = ShaderStorageBuffer(SSBO_POINT_LIGHT_DATA_ARRAY, MAX_POINT_LIGHTS * sizeof(PointLightData));
	
	cameraDataUBO = UniformBuffer(UBO_CAMERA_DATA, sizeof(CameraData));
	sceneLightDataUBO = UniformBuffer(UBO_SCENE_LIGHT_DATA, sizeof(SceneLightData), GL_DYNAMIC_STORAGE_BIT);
	cascadeDataUBO = UniformBuffer(UBO_SHADOW_CASCADE_DATA, SHADOW_MAP_MAX_CASCADES * sizeof(CascadeData), GL_DYNAMIC_STORAGE_BIT);

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

	InitGBuffer();
	InitShadowMapFBOs();
	InitLightingFBO();
	InitPostFxFBO();
	InitSSAOUniformBuffer();

	glEnable(GL_CULL_FACE);

	return true;
}

void OpenGL::InitSSAOUniformBuffer() {

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	SSAONoiseTex2D = Texture2D("ssao_noise_tex2D", GL_RGBA16F, 4, 4);
	SSAONoiseTex2D.SetSubImage2D(GL_RGBA, 0, 0, 0, GL_FLOAT, ssaoNoise.data());
	SSAONoiseTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSAONoiseTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	SSAONoiseTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	SSAONoiseTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_REPEAT);

	SSAOSettingsData ssaoSettings;
	ssaoSettings.noiseScale = glm::vec2{ SCR_WIDTH / 4.0f, SCR_HEIGHT / 4.0f };
	
	//std::vector<glm::vec4> ssaoKernel;
	for (uint32_t i = 0; i < SSAO_KERNEL_SIZE; i++) {
		glm::vec4 sample = {
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator),
			1.0
		};
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);

		float scale = (float)i / SSAO_KERNEL_SIZE;
		scale = GLUtils::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		//ssaoKernel.push_back(sample);
		ssaoSettings.samples[i] = sample;
	}

	ssaoSettingsDataUBO = UniformBuffer(UBO_SSAO_SETTINGS_DATA, sizeof(SSAOSettingsData), GL_DYNAMIC_STORAGE_BIT);
	ssaoSettingsDataUBO.UpdateData(0, sizeof(SSAOSettingsData), &ssaoSettings);

	SSAOTex2D = Texture2D("gSSAO_tex2D", GL_R16F, SCR_WIDTH, SCR_HEIGHT);
	SSAOTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	SSAOTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSAOTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	SSAOTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::vector<float> noSSAO(SCR_WIDTH * SCR_HEIGHT, 1.0f);
	SSAOTex2D.SetSubImage2D(GL_RED, 0, 0, 0, GL_FLOAT, noSSAO.data());

	ssaoFBO = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	ssaoFBO.CreateRenderBuffer();
	ssaoFBO.AttachColorTex2D(SSAOTex2D.GetID(), 0);
	ssaoFBO.DrawColorBuffers();

	if (!ssaoFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "SSAO frame buffer is incomplete.");
	}

	SSAOBlurTex2D = Texture2D("gSSAO_blurred_tex2d", GL_R16F, SCR_WIDTH, SCR_HEIGHT);
	SSAOBlurTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	SSAOBlurTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	ssaoBlurFBO = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	ssaoBlurFBO.CreateRenderBuffer();
	ssaoBlurFBO.AttachColorTex2D(SSAOBlurTex2D.GetID(), 0);
	ssaoBlurFBO.DrawColorBuffers();

	if (!ssaoBlurFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "SSAO Blurred frame buffer is incomplete.");
	}
}

void OpenGL::InitPostFxFBO() {
	postFXTex2D = Texture2D("post_fx_tex2d", GL_RGB8, SCR_WIDTH, SCR_HEIGHT);
	postFXTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	postFXTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	postfxFBO = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	postfxFBO.CreateRenderBuffer();
	postfxFBO.AttachColorTex2D(postFXTex2D.GetID(), 0);

	if (!postfxFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Post FX frame buffer is incomplete.");
	}
}

void OpenGL::InitShadowMapFBOs() {
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float invBorderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	// point shadow map
	BIND(Shader, pointLightShadowMapShader); // we bind here to tell open gl that we are using a geometry shader
	pointShadowFBO = FrameBuffer(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	pointShadowCubemapArray = TextureCubeMapArray("point_light_shadow_cubemap_array", GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 6 * MAX_POINT_LIGHTS, 1);
	pointShadowCubemapArray.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);
	pointShadowFBO.AttachDepthCubeMapTex(pointShadowCubemapArray.GetID());
	pointShadowFBO.DisableColorBuffer();

	if (!pointShadowFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Point light shadow map frame buffer is incomplete.");
		return;
	}

	// Gaussian blur on shadows
	vBlurShadowMapTex2D = Texture2D("vBlurShadowMapTex2D", GL_RGB16F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	vBlurShadowMapTex2D.SetParams(GL_TEXTURE_BORDER_COLOR, invBorderColor);

	vBlurShadowMapFBO = FrameBuffer(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	vBlurShadowMapFBO.CreateRenderBuffer();
	vBlurShadowMapFBO.AttachColorTex2D(vBlurShadowMapTex2D.GetID(), 0);

	hBlurShadowMapTex2D = Texture2D("hBlurShadowMapTex2D", GL_RGB16F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	hBlurShadowMapTex2D.SetParams(GL_TEXTURE_BORDER_COLOR, invBorderColor);

	hBlurShadowMapFBO = FrameBuffer(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	hBlurShadowMapFBO.AttachColorTex2D(hBlurShadowMapTex2D.GetID(), 0);
	hBlurShadowMapFBO.CreateRenderBuffer();

	if (!hBlurShadowMapFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Horizontal pass shadow map blur buffer is incomplete.");
		return;
	}
	
	if (!vBlurShadowMapFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Vertical pass shadow map blur buffer is incomplete.");
		return;
	}

	// CSM Shadow Maps
	shadowMapTex2DArray = Texture2DArray("shadow_map_tex2d", GL_RG32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, SHADOW_MAP_N_CASCADES);
	shadowMapTex2DArray.SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	shadowMapTex2DArray.SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	shadowMapTex2DArray.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	shadowMapTex2DArray.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	shadowMapTex2DArray.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);

	Texture2DArray shadowMapDepthTex2DArray = Texture2DArray("shadow_map_depth_tex2darray", GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, SHADOW_MAP_N_CASCADES);
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	shadowMapDepthTex2DArray.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);

	BIND(Shader, shadowMapShader); // we bind here to tell open gl that we are using a geometry shader
	shadowMapFBO = FrameBuffer(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
	shadowMapFBO.AttachDepthTex2D(shadowMapDepthTex2DArray.GetID());
	shadowMapFBO.AttachColorTex2D(shadowMapTex2DArray.GetID(), 0);
	shadowMapFBO.DrawBuffer(0);

	if (!shadowMapFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Shadow map frame buffer is incomplete.");
		return;
	}
}

void OpenGL::InitLightingFBO()
{
	diffuseTex2D = Texture2D("gDiffuse_tex2D", GL_RGB16F, SCR_WIDTH, SCR_HEIGHT);
	diffuseTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	diffuseTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	lightingFBO = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	lightingFBO.CreateRenderBuffer();
	lightingFBO.AttachColorTex2D(diffuseTex2D.GetID(), 0);

	if (!lightingFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Lighting frame buffer is incomplete.");
	}
}

void OpenGL::InitGBuffer() {
	gPosition = Texture2D("gPosition_tex2D", GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT);
	gPosition.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gPosition.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	gViewPosition = Texture2D("gViewPosition_tex2D", GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT);
	gViewPosition.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gViewPosition.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gNormal = Texture2D("gNormal_tex2D", GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT);
	gNormal.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gNormal.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	gViewNormal = Texture2D("gViewNormal_tex2D", GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT);
	gViewNormal.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gViewNormal.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gAlbedo = Texture2D("gDiffuse_tex2D", GL_RGB8, SCR_WIDTH, SCR_HEIGHT);
	gAlbedo.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gAlbedo.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gSpecular = Texture2D("gSpecular_tex2D", GL_R8, SCR_WIDTH, SCR_HEIGHT);
	gSpecular.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gSpecular.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gMetallic = Texture2D("gMetallic_tex2D", GL_R8, SCR_WIDTH, SCR_HEIGHT);
	gMetallic.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gMetallic.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gBufferFBO = FrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	gBufferFBO.AttachColorTex2D(gPosition.GetID(), 0);
	gBufferFBO.AttachColorTex2D(gNormal.GetID(), 1);
	gBufferFBO.AttachColorTex2D(gAlbedo.GetID(), 2);
	gBufferFBO.AttachColorTex2D(gSpecular.GetID(), 3);
	gBufferFBO.AttachColorTex2D(gMetallic.GetID(), 4);
	gBufferFBO.AttachColorTex2D(gViewPosition.GetID(), 5);
	gBufferFBO.AttachColorTex2D(gViewNormal.GetID(), 6);
	gBufferFBO.DrawColorBuffers();
	gBufferFBO.CreateRenderBuffer();

	if (!gBufferFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Gbuffer frame buffer is incomplete.");
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
	glTex2D.SetHandleIndex(static_cast<int32_t>(tex2DHndlrDataArray.size()));

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
	sceneLightData.mAmbientLightIntensity = .4f;
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

void OpenGL::PointLightShadowMapPass() {
	if (pointLightDataArray.empty())
		return;

	BIND(FrameBuffer, pointShadowFBO);
	pointShadowFBO.SetViewport();

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	size_t pointLightsCount = pointLightDataArray.size();

	BIND(Shader, pointLightShadowMapShader);
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

		pointLightShadowMapShader.SetUMat4v("uCubeMapMatrices", static_cast<GLsizei>(views.size()), views.data());
		pointLightShadowMapShader.SetUVec3("uLightPos", lightData.mPosition);
		pointLightShadowMapShader.SetUFloat("uLightFarPlane", lightData.shadowFarPlane);
		pointLightShadowMapShader.SetUInt("uBaseLayerOffset", 6*i);

		pointLightDataArraySSBO.UpdateData(lightData.dataArrayIndex * sizeof(PointLightData) + offsetof(PointLightData, shadowCubeMapIndex), sizeof(GLuint), &lightData.shadowCubeMapIndex);

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(meshDrawCmdDataArray.size()), sizeof(MeshDrawCmdData));
	}

	glCullFace(GL_BACK);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void OpenGL::RegisterCamera(Camera* camera) {
	if (camera != nullptr) {
		currentActiveCamera = camera;

		cascadeDataArray.clear();
		cascadeDataArray = std::vector<CascadeData>(SHADOW_MAP_N_CASCADES);

		for (int i = 0; i < SHADOW_MAP_N_CASCADES; i++) {
			
			const float fractionNear = static_cast<float>(i) / static_cast<float>(SHADOW_MAP_N_CASCADES);
			const float fractionFar = static_cast<float>(i + 1) / static_cast<float>(SHADOW_MAP_N_CASCADES);

			const float cameraClipRange = camera->mFarClipPlane - camera->mNearClipPlane;
			const float cameraClipRatio = camera->mFarClipPlane / camera->mNearClipPlane;

			const float uniformNear = camera->mNearClipPlane + cameraClipRange * fractionNear;
			const float uniformFar = camera->mNearClipPlane + cameraClipRange * fractionFar;

			const float logNear = camera->mNearClipPlane * std::pow(cameraClipRatio, fractionNear);
			const float logFar = camera->mNearClipPlane * std::pow(cameraClipRatio, fractionFar);

			const float blend = 0.1f;
			cascadeDataArray[i].nearPlane = blend * uniformNear + (1.f - blend) * logNear;
			cascadeDataArray[i].farPlane = blend * uniformFar + (1.f - blend) * logFar;
		}

		cascadeDataArray[0].nearPlane = camera->mNearClipPlane;
		cascadeDataArray[SHADOW_MAP_N_CASCADES - 1].farPlane = camera->mFarClipPlane;
	}
}

void OpenGL::ShadowMapPass() {
	if (directionalLight == nullptr) return;

	for (int i = 0; i < SHADOW_MAP_N_CASCADES; i++) {
		const float cascadeNearPlane = cascadeDataArray[i].nearPlane;
		const float cascadeFarPlane = cascadeDataArray[i].farPlane;

		const std::vector<glm::vec4> corners = GLUtils::GetFrustumCornersWorldSpace(currentActiveCamera->mFOV, currentActiveCamera->mAspectRatio, cascadeNearPlane, cascadeFarPlane, currentActiveCamera->ViewMatrix());
		const glm::mat4 view = GLUtils::GetLightViewMatrix(directionalLight->direction, corners);
		const glm::mat4 lightSpaceMatrix = GLUtils::GetLightSpaceMatrix(view, corners);
		cascadeDataArray[i].lightSpaceMatrix = lightSpaceMatrix;
	}

	cascadeDataUBO.UpdateData(0, cascadeDataArray.size() * sizeof(CascadeData), cascadeDataArray.data());

	BIND(Shader, shadowMapShader);
	BIND(FrameBuffer, shadowMapFBO);
	BIND(DrawIndirectBuffer, meshDIB);
	BIND(VertexArray, meshVAO);

	shadowMapFBO.SetViewport();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(meshDrawCmdDataArray.size()), sizeof(MeshDrawCmdData));

	glCullFace(GL_BACK);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void OpenGL::LightingPass() {
	BIND_TEX(TextureCubeMapArray, pointShadowCubemapArray, 0);
	BIND_TEX(Texture2DArray, shadowMapTex2DArray, 1);
	BIND_TEX(Texture2D, gPosition, 2);
	BIND_TEX(Texture2D, gNormal, 3);
	BIND_TEX(Texture2D, gAlbedo, 4);
	BIND_TEX(Texture2D, gSpecular, 5);
	BIND_TEX(Texture2D, gMetallic, 6);
	BIND_TEX(Texture2D, SSAOBlurTex2D, 7);
	BIND(Shader, lightingShader);
	BIND(FrameBuffer, lightingFBO);
	BIND(VertexArray, screenQuadVAO);

	lightingFBO.SetViewport();
	lightingShader.SetUInt("cascadeCount", static_cast<uint32_t>(cascadeDataArray.size()));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
}

void OpenGL::OutputToScreen(const SceneTexture st) {
	Texture2D sceneTexture;
	switch (st) {
		case SceneTexture::ST_POST_FX:
			sceneTexture = postFXTex2D;
			break;
		case SceneTexture::ST_DIFFUSE:
			sceneTexture = diffuseTex2D;
			break;
		case SceneTexture::ST_ALBEDO:
			sceneTexture = gAlbedo;
			break;
		case SceneTexture::ST_METALLIC:
			sceneTexture = gMetallic;
			break;
		case SceneTexture::ST_SPECULAR:
			sceneTexture = gSpecular;
			break;
		case SceneTexture::ST_WORLD_NORMAL:
			sceneTexture = gNormal;
			break;
		case SceneTexture::ST_WORLD_POSITION:
			sceneTexture = gPosition;
			break;
		case SceneTexture::ST_SSAO:
			sceneTexture = SSAOTex2D;
			break;
		default:
			LOG(LogOpenGL, LOG_CRITICAL, std::format("Invalid scene texture selected."));
	}

	BIND(VertexArray, screenQuadVAO);
	BIND_TEX(Texture2D, sceneTexture, 0);
	BIND(Shader, screenShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void OpenGL::PostFxPass()
{
	BIND_TEX(Texture2D, diffuseTex2D, 0);
	BIND(Shader, postFxShader);
	BIND(VertexArray, screenQuadVAO);
	BIND(FrameBuffer, postfxFBO);

	postFxShader.SetUFloat("uGamma", GAMMA);
	postfxFBO.SetViewport();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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
		auto [entInstIt, inserted] = assToEntMeshInstIndexes[mesh->assetId].try_emplace(entity, static_cast<uint32_t>(meshInstances.size()));

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
		it->second.dataArrayIndex = static_cast<GLuint>(pointLightDataArray.size());
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
		it->second.drawCmdIndex = static_cast<int32_t>(meshDrawCmdDataArray.size());

		meshDrawCmdDataArray.push_back(MeshDrawCmdData());
		MeshDrawCmdData& cmd = meshDrawCmdDataArray.back();

		cmd.count = static_cast<GLuint>(indices.size());
		cmd.instanceCount = 0;
		cmd.firstIndex = static_cast<GLuint>(meshIndexDataArray.size());
		cmd.baseVertex = static_cast<GLuint>(meshVertexDataArray.size());

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
		cmdData.baseInstance = static_cast<GLuint>(meshInstDataArray.size());

		const std::vector<MeshInstanceData>& meshInsts = it.second;
		meshInstDataArray.insert(meshInstDataArray.end(), meshInsts.begin(), meshInsts.end());
	}

	meshDIB.UpdateData(0, meshDrawCmdDataArray.size() * sizeof(MeshDrawCmdData), meshDrawCmdDataArray.data());
	meshSSBO.UpdateData(0, meshInstDataArray.size() * sizeof(MeshInstanceData), meshInstDataArray.data());
}

void OpenGL::GeometryPass() {
	BIND(Shader, gBufferShader);
	BIND(FrameBuffer, gBufferFBO);
	BIND(DrawIndirectBuffer, meshDIB);
	BIND(VertexArray, meshVAO);

	gBufferFBO.SetViewport();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(meshDrawCmdDataArray.size()), sizeof(MeshDrawCmdData));
}

void OpenGL::SSAOPass() {
	BIND(VertexArray, screenQuadVAO);

	{
		BIND_TEX(Texture2D, gViewPosition, 0);
		BIND_TEX(Texture2D, gViewNormal, 1);
		BIND_TEX(Texture2D, SSAONoiseTex2D, 2);
		BIND(Shader, ssaoShader);
		BIND(FrameBuffer, ssaoFBO);
		
		ssaoFBO.SetViewport();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	}

	{
		BIND_TEX(Texture2D, SSAOTex2D, 0);
		BIND(Shader, ssaoBlurShader);
		BIND(FrameBuffer, ssaoBlurFBO);

		ssaoBlurFBO.SetViewport();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	}

}
