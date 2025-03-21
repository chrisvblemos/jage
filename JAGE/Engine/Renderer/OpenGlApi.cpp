#include <ECS/EntityManager.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Components/PointLight.h>
#include <ECS/Components/StaticMeshRenderer.h>
#include <Core/Assets/AssetManager.h>
#include <Core/Log/LogDisplay.h>
#include <Core/Config.h>
#include "ShaderPreProcessor.h"
#include "Core/Utils.h"
#include <physx/PxPhysicsAPI.h>
#include "OpenGlApi.h"

using namespace physx;

void EnableOpenGLDebugOutput();

bool OpenGlApi::Initialize() {

	EnableOpenGLDebugOutput();
	ShaderPreProcessor spp = ShaderPreProcessor();
	spp.Initialize();
	
	lightingShader            = Shader(spp.GetCodeStr("lighting.vert"), 
									   spp.GetCodeStr("lighting.frag"));
	screenShader              = Shader(spp.GetCodeStr("screen.vert"), 
						  			   spp.GetCodeStr("screen.frag"));
	gBufferShader             = Shader(spp.GetCodeStr("gbuffer.vert"),
						               spp.GetCodeStr("gbuffer.frag"));
	pointLightShadowMapShader = Shader(spp.GetCodeStr("point_shadow_map.vert"),
									   spp.GetCodeStr("point_shadow_map.frag"),
									   spp.GetCodeStr("point_shadow_map.geom"));
	varianceShadowMapShader   = Shader(spp.GetCodeStr("vsm.vert"),
									   spp.GetCodeStr("vsm.frag"));
	hBlurShadowMapShader      = Shader(spp.GetCodeStr("screen.vert"),
								       spp.GetCodeStr("gaussian_blur_h.frag"));
	vBlurShadowMapShader      = Shader(spp.GetCodeStr("screen.vert"),
								       spp.GetCodeStr("gaussian_blur_v.frag"));
	shadowMapShader           = Shader(spp.GetCodeStr("csm.vert"),
							           spp.GetCodeStr("csm.frag"),
							           spp.GetCodeStr("csm.geom"));
	ssaoShader                = Shader(spp.GetCodeStr("ssao.vert"),
					                   spp.GetCodeStr("ssao.frag"));
	ssaoBlurShader            = Shader(spp.GetCodeStr("ssao.vert"),
							           spp.GetCodeStr("ssao_blur.frag"));
	postFxShader              = Shader(spp.GetCodeStr("screen.vert"),
						               spp.GetCodeStr("post_fx.frag"));

	meshVBO = VertexArrayBuffer(
		5000 * cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.max_meshes", 10000), 
		GL_DYNAMIC_STORAGE_BIT);
	meshEBO = ElementArrayBuffer(
		3 * 5000 * cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.max_meshes", 10000), 
		GL_DYNAMIC_STORAGE_BIT);
	meshDIB = DrawIndirectBuffer(
		SSBO_MESH_INDIRECT_DRAW_COMMAND, 
		cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.max_meshes", 10000) * sizeof(MeshDrawCmdData), 
		GL_DYNAMIC_STORAGE_BIT);

	meshSSBO = ShaderStorageBuffer(
		SSBO_MESH_INSTANCE_DATA, 
		cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.max_meshes", 10000) * sizeof(MeshInstanceData));
	textureHndlrsSSBO = ShaderStorageBuffer(
		SSBO_TEXTURE_HANDLERS, 
		cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.max_textures", 10000) * sizeof(GLuint64));
	pointLightDataArraySSBO = ShaderStorageBuffer(
		SSBO_POINT_LIGHT_DATA_ARRAY, 
		cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.max_point_lights", 8) * sizeof(PointLightData));
	
	cameraDataUBO     = UniformBuffer(UBO_CAMERA_DATA, sizeof(CameraData));
	sceneLightDataUBO = UniformBuffer(UBO_SCENE_LIGHT_DATA, sizeof(SceneLightData), GL_DYNAMIC_STORAGE_BIT);
	cascadeDataUBO    = UniformBuffer(UBO_SHADOW_CASCADE_DATA, SHADOW_MAP_MAX_CASCADES * sizeof(CascadeData), GL_DYNAMIC_STORAGE_BIT);

	meshVAO = VertexArray();
	std::vector<VertexAttrib> meshVAOAttribs = {
		{ 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position) },
		{ 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal) },
		{ 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords) }
	};
	meshVAO.Configure(meshVBO.GetID(), sizeof(Vertex), meshEBO.GetID(), meshVAOAttribs);


	screenQuadVBO = VertexArrayBuffer(sizeof(quadVertices), GL_DYNAMIC_STORAGE_BIT);
	screenQuadEBO = ElementArrayBuffer(sizeof(quadIndices), GL_DYNAMIC_STORAGE_BIT);
	screenQuadVBO.UpdateData(0, sizeof(quadVertices), quadVertices);
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

	SetFaceCullEnabled(true);
	SetDepthEnabled(true);

	return true;
}

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam) {
	const char* sourceStr;
	const char* typeStr;
	const char* severityStr;

	// Determine the source of the message
	switch (source) {
	case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
	default:                              sourceStr = "Unknown"; break;
	}

	// Determine the type of the message
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability Issue"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance Issue"; break;
	case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
	default:                                typeStr = "Unknown"; break;
	}

	// Determine the severity of the message
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
	default:                             severityStr = "Unknown"; break;
	}

	std::string msg = std::format("Source: {}, Type: {}, ID: {}, Message: {}", sourceStr, type, id, message);

	// Print debug message
	//std::cerr << "[OpenGL Debug] Source: " << sourceStr
	//	<< ", Type: " << typeStr
	//	<< ", Severity: " << severityStr
	//	<< ", ID: " << id
	//	<< ", Message: " << message << std::endl;

	if (type == GL_DEBUG_TYPE_ERROR) {
		LOG(LogGeneric, LOG_ERROR, msg);
	}
	else {
		LOG(LogGeneric, LOG_VERBOSE, msg);
	}

#ifdef _DEBUG
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		__debugbreak(); // Break into debugger on high-severity errors (Visual Studio only)
	}
#endif
}

// Initialize OpenGL debug output
void EnableOpenGLDebugOutput() {
	glEnable(GL_DEBUG_OUTPUT); // Enable debug output
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Make it synchronous for easier debugging
	glDebugMessageCallback(GLDebugMessageCallback, nullptr); // Register callback function

	// Optionally control which messages to log (filtering)
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

/* Projection matrix uses camera fov and aspect ratio, and the near and far plane
of the frustum in question. The view matrix is the camera's view matrix as usual. */
std::vector<Vec4> OpenGlApi::GetFrustumCornersWorldSpace(const float fov, const float aspectRatio, const float nearPlane, const float farPlane, const Mat4& view) {
	const Mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	const auto inv = glm::inverse(proj * view);

	std::vector<Vec4> corners;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				const Vec4 point =
					inv * Vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				corners.push_back(point / point.w);
			}
		}
	}

	return corners;
}

Mat4 OpenGlApi::GetLightViewMatrix(const Vec3& lightDir, const std::vector<Vec4>& frustumCorners) {
	Vec3 center = Vec3(0.0f); // center of light frustum
	for (const auto& v : frustumCorners) {
		center += Vec3(v);
	}

	center /= (float)frustumCorners.size();

	Vec3 normalizedLightDir = glm::normalize(lightDir);
	float pitch = glm::asin(normalizedLightDir.y);
	float yaw = glm::atan(normalizedLightDir.x, normalizedLightDir.z);

	Mat4 view = Mat4(1.0f);
	Mat4 translate = glm::translate(view, -(center - lightDir));
	Mat4 rotYaw = glm::rotate(view, -yaw, { 0.0f, 1.0f, 0.0f });
	Mat4 rotPitch = glm::rotate(view, -pitch, { 1.0f, 0.0f, 0.0f });
	return rotPitch * rotYaw * translate;
}

void OpenGlApi::UpdateCameraFrustum(const Transform* transform)
{
	float aspect = currentActiveCamera->aspectRatio;
	float zNear = currentActiveCamera->nearPlane;
	float zFar = currentActiveCamera->farPlane;

	const float halfVSide = zFar * tanf(currentActiveCamera->fov * .5f);
	const float halfHSide = halfVSide * currentActiveCamera->aspectRatio;
	const Vec3 frontMultNear = zNear * transform->forward;
	const Vec3 frontMultFar = zFar * transform->forward;

	cameraFrustum.nearFace = { transform->position + frontMultNear, transform->forward };
	cameraFrustum.farFace = { transform->position + frontMultFar, -transform->forward };
	cameraFrustum.rightFace = { transform->position,
							glm::cross(transform->up, frontMultFar - transform->right * halfHSide) };
	cameraFrustum.leftFace = { transform->position,
							glm::cross(frontMultFar + transform->right * halfHSide, transform->up) };
	cameraFrustum.topFace = { transform->position,
							glm::cross(frontMultFar - transform->up * halfVSide, transform->right) };
	cameraFrustum.bottomFace = { transform->position,
							glm::cross(transform->right, frontMultFar + transform->up * halfVSide) };
}

/* Returns the light proj matrix when using CSM. To get the view-proj matrix,
just multiply by the view matrix (see GetLightViewMatrix()).*/
Mat4 OpenGlApi::GetLightSpaceMatrix(const Mat4& lightViewMatrix, const std::vector<Vec4>& corners) {
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	for (const auto& v : corners)
	{
		const auto trf = lightViewMatrix * v;
		minX = glm::min(minX, trf.x);
		maxX = glm::max(maxX, trf.x);
		minY = glm::min(minY, trf.y);
		maxY = glm::max(maxY, trf.y);
		minZ = glm::min(minZ, trf.z);
		maxZ = glm::max(maxZ, trf.z);
	}

	//auto tmp = -minZ;
	//minZ = -maxZ;
	//maxZ = tmp;

	//auto mid = (maxZ - minZ) / 2;
	//minZ -= mid * 5.0f;
	//maxZ += mid * 5.0f;

	constexpr float zMult = 10.0f; // multiplier to extend light view frustum to 
	// corners of camera (objects out of view still cast shadows)
	if (minZ < 0)
		minZ *= zMult;
	else
		minZ /= zMult;

	if (maxZ < 0)
		maxZ /= zMult;
	else
		maxZ *= zMult;

	//auto mid = (maxZ - minZ) / 2.0;
	//minZ -= mid * 5.0f;
	//maxZ += mid * 5.0f;

	const Mat4 lightProjection = glm::ortho(
		minX,
		maxX,
		minY,
		maxY,
		minZ,
		maxZ
	);

	return lightProjection * lightViewMatrix;
};

void OpenGlApi::InitSSAOUniformBuffer() {

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	std::vector<Vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		Vec3 noise(
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
	ssaoSettings.kernelSize = SSAO_KERNEL_SIZE;
	ssaoSettings.noiseScale = glm::vec2{ 
		cfg::Rendering.Read<float>("Video", "video.resX", 800.0f) / 4.0f, 
		cfg::Rendering.Read<float>("Video", "video.resY", 600.0f) / 4.0f };
	ssaoSettings.bias = cfg::Rendering.Read<float>("OpenGL", "opengl.ssao.bias", 0.025f);
	ssaoSettings.power = cfg::Rendering.Read<float>("OpenGL", "openg.ssao.power", 1.0f);
	ssaoSettings.radius = cfg::Rendering.Read<float>("OpenGL", "opengl.ssao.radius", 0.5f);

	//std::vector<Vec4> ssaoKernel;
	for (uint32_t i = 0; i < SSAO_KERNEL_SIZE; i++) {
		Vec4 sample = {
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator),
			1.0
		};
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);

		float scale = (float)i / SSAO_KERNEL_SIZE;
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		sample *= scale;
		//ssaoKernel.push_back(sample);
		ssaoSettings.samples[i] = sample;
	}

	ssaoSettingsDataUBO = UniformBuffer(UBO_SSAO_SETTINGS_DATA, sizeof(SSAOSettingsData), GL_DYNAMIC_STORAGE_BIT);
	ssaoSettingsDataUBO.UpdateData(0, sizeof(SSAOSettingsData), &ssaoSettings);

	SSAOTex2D = Texture2D("gSSAO_tex2D", 
						  GL_R16F, 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	SSAOTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	SSAOTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSAOTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
	SSAOTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::vector<float> noSSAO(cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800) * 
							  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600), 
							  1.0f);
	SSAOTex2D.SetSubImage2D(GL_RED, 0, 0, 0, GL_FLOAT, noSSAO.data());

	ssaoFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	ssaoFBO.CreateRenderBuffer();
	ssaoFBO.AttachColorTex2D(SSAOTex2D.GetID(), 0);
	ssaoFBO.DrawColorBuffers();

	if (!ssaoFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "SSAO frame buffer is incomplete.");
	}

	SSAOBlurTex2D = Texture2D("gSSAO_blurred_tex2d", 
							  GL_R16F, 
							  cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
							  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	SSAOBlurTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	SSAOBlurTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	ssaoBlurFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
							  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	ssaoBlurFBO.CreateRenderBuffer();
	ssaoBlurFBO.AttachColorTex2D(SSAOBlurTex2D.GetID(), 0);
	ssaoBlurFBO.DrawColorBuffers();

	if (!ssaoBlurFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "SSAO Blurred frame buffer is incomplete.");
	}
}

void OpenGlApi::InitPostFxFBO() {
	postFXTex2D = Texture2D("post_fx_tex2d", 
							GL_RGB8, 
							cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
							cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	postFXTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	postFXTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	postfxFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
							cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	postfxFBO.CreateRenderBuffer();
	postfxFBO.AttachColorTex2D(postFXTex2D.GetID(), 0);

	if (!postfxFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Post FX frame buffer is incomplete.");
	}
}

void OpenGlApi::InitShadowMapFBOs() {
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float invBorderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	// point shadow map
	BIND(Shader, pointLightShadowMapShader); // we bind here to tell open gl that we are using a geometry shader
	pointShadowFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
								 cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512));
	pointShadowCubemapArray = TextureCubeMapArray("point_light_shadow_cubemap_array", 
												  GL_DEPTH_COMPONENT32F, 
												  cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
												  cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
												  6 * cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.max_point_lights", 8), 1);
	pointShadowCubemapArray.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);
	pointShadowFBO.AttachDepthCubeMapTex(pointShadowCubemapArray.GetID());
	pointShadowFBO.DisableColorBuffer();

	if (!pointShadowFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Point light shadow map frame buffer is incomplete.");
		return;
	}

	// Gaussian blur on shadows
	vBlurShadowMapTex2D = Texture2D("vBlurShadowMapTex2D", 
						            GL_RGB16F, 
									cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
									cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512));
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	vBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	vBlurShadowMapTex2D.SetParams(GL_TEXTURE_BORDER_COLOR, invBorderColor);

	vBlurShadowMapFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
									cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512));
	vBlurShadowMapFBO.CreateRenderBuffer();
	vBlurShadowMapFBO.AttachColorTex2D(vBlurShadowMapTex2D.GetID(), 0);

	hBlurShadowMapTex2D = Texture2D("hBlurShadowMapTex2D", 
									GL_RGB16F, 
									cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
									cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512));
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	hBlurShadowMapTex2D.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	hBlurShadowMapTex2D.SetParams(GL_TEXTURE_BORDER_COLOR, invBorderColor);

	hBlurShadowMapFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
									cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512));
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
	shadowMapTex2DArray = Texture2DArray("shadow_map_tex2d", 
										 GL_RG32F, 
										 cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
										 cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.cascade_count", 4));
	shadowMapTex2DArray.SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	shadowMapTex2DArray.SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	shadowMapTex2DArray.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	shadowMapTex2DArray.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	shadowMapTex2DArray.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);

	Texture2DArray shadowMapDepthTex2DArray = Texture2DArray("shadow_map_depth_tex2darray", 
															 GL_DEPTH_COMPONENT32F, 
															 cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
															 cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
															 cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.cascade_count", 4));
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	shadowMapDepthTex2DArray.SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	shadowMapDepthTex2DArray.SetParams(GL_TEXTURE_BORDER_COLOR, borderColor);

	BIND(Shader, shadowMapShader); // we bind here to tell open gl that we are using a geometry shader
	shadowMapFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512), 
						       cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.resolution", 512));
	shadowMapFBO.AttachDepthTex2D(shadowMapDepthTex2DArray.GetID());
	shadowMapFBO.AttachColorTex2D(shadowMapTex2DArray.GetID(), 0);
	shadowMapFBO.DrawBuffer(0);

	if (!shadowMapFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Shadow map frame buffer is incomplete.");
		return;
	}
}

void OpenGlApi::InitLightingFBO()
{
	diffuseTex2D = Texture2D("gDiffuse_tex2D", 
							 GL_RGB16F, 
							 cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
							 cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	diffuseTex2D.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	diffuseTex2D.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	lightingFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
							  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	lightingFBO.CreateRenderBuffer();
	lightingFBO.AttachColorTex2D(diffuseTex2D.GetID(), 0);

	if (!lightingFBO.CheckComplete()) {
		LOG(LogOpenGL, LOG_CRITICAL, "Lighting frame buffer is incomplete.");
	}
}

void OpenGlApi::InitGBuffer() {
	gPosition = Texture2D("gPosition_tex2D", 
						  GL_RGBA16F, 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	gPosition.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gPosition.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	gViewPosition = Texture2D("gViewPosition_tex2D", 
						  	  GL_RGBA16F, 
						  	  cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						  	  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	gViewPosition.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gViewPosition.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gNormal = Texture2D("gNormal_tex2D", 
						GL_RGBA16F, 
						cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	gNormal.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gNormal.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	gViewNormal = Texture2D("gViewNormal_tex2D", 
						    GL_RGBA16F, 
						    cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						    cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	gViewNormal.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gViewNormal.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gAlbedo = Texture2D("gDiffuse_tex2D", 
						GL_RGB8, 
						cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	gAlbedo.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gAlbedo.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gSpecular = Texture2D("gSpecular_tex2D", 
						  GL_R8, 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	gSpecular.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gSpecular.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gMetallic = Texture2D("gMetallic_tex2D", 
						  GL_R8, 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						  cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
	gMetallic.SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gMetallic.SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gBufferFBO = FrameBuffer(cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
						     cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));
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

void OpenGlApi::RegisterTexture2D(Texture* texture) {
	if (texture == nullptr)
		LOG(LogOpenGL, LOG_CRITICAL, std::format("Failed to register texture {} to buffer.", texture->assetPath));

	auto it = assetIDToTex2DMap.find(texture->assetId);
	if (it != assetIDToTex2DMap.end())
		return;
	
	GLenum internalFormat;
	if (texture->type == TextureAssetType::Albedo) {
		if (texture->hasAlphaChannel) {
			if (texture->inLinearSpace)
				internalFormat = GL_RGBA8;
			else
				internalFormat = GL_SRGB8_ALPHA8;
		}
		else {
			if (texture->inLinearSpace)
				internalFormat = GL_RGB8;
			else
				internalFormat = GL_SRGB8;
		}
	}
	else if (texture->type == TextureAssetType::Metallic || 
			 texture->type == TextureAssetType::Specular ||
			 texture->type == TextureAssetType::Roughness ||
			 texture->type == TextureAssetType::AO ||
			 texture->type == TextureAssetType::Linear) {
		internalFormat = GL_R8;
	}
	else if (texture->type == TextureAssetType::Normal) {
		internalFormat = GL_RG8;
	}
	else {
		LOG(LogOpenGL, LOG_CRITICAL, std::format("Failed to infer texture format for {}", texture->assetPath));
	}
	GLenum format = internalToFormat(internalFormat);

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

void OpenGlApi::UploadSceneLightData() {
	SceneLightData sceneLightData;
	sceneLightData.mHasDirectionalLight = static_cast<GLuint>(directionalLight != nullptr);
	sceneLightData.mDirectionalLightColor = directionalLight? directionalLight->color : Vec3(1.0f);
	sceneLightData.mDirectionalLightDirection = directionalLight? directionalLight->direction : Vec3(1.0f);
	sceneLightData.mDirectionalLightIntensity = directionalLight? directionalLight->intensity : 1.0f;
	sceneLightData.mDirectionalLightMatrix = directionalLight? directionalLight-> projMatrix * directionalLight->viewMatrix : Mat4(1.0f);
	sceneLightData.mAmbientLightColor = Vec3(1.0f);
	sceneLightData.mAmbientLightIntensity = 0.6f;
	sceneLightData.mPointLightsCount = static_cast<GLsizei>(pointLightDataArray.size());

	sceneLightDataUBO.UpdateData(0, sizeof(SceneLightData), &sceneLightData);
}

void OpenGlApi::UploadCameraData(const Camera* camera, const Transform* transform) {
	assert(currentActiveCamera && "OpenGL: Attempting to upload camera data without an active camera.");

	CameraData cameraData = {};
	cameraData.mPosition = Vec4(transform->position, 1.0f);
	cameraData.mProjection = currentActiveCamera->projMatrix;
	cameraData.mView = currentActiveCamera->viewMatrix;

	cameraDataUBO.UpdateData(0, sizeof(CameraData), &cameraData);
	UpdateCameraFrustum(transform);
}

void OpenGlApi::PointLightShadowMapPass() {
	if (pointLightDataArray.empty())
		return;

	BIND(FrameBuffer, pointShadowFBO);
	BIND(Shader, pointLightShadowMapShader);
	BIND(VertexArray, meshVAO);
	BIND(DrawIndirectBuffer, meshDIB);
	ClearBuffers(GL_DEPTH_BUFFER_BIT);
	SetDepthEnabled(true);
	SetFaceCullMode(GL_FRONT);
	SetViewport(0, 0, pointShadowFBO.GetWidth(), pointShadowFBO.GetHeight());
	size_t pointLightsCount = pointLightDataArray.size();

	// we re-draw the scene for each of the faces
	// of the cube map depth texture
	for (unsigned int i = 0; i < pointLightsCount; i++) {
		pointLightShadowMapShader.SetUInt("uPointLightIndex", i);
		glMultiDrawElementsIndirect(GL_TRIANGLES,
									GL_UNSIGNED_INT,
									nullptr,
									static_cast<GLsizei>(meshDrawCmdDataArray.size()),
									sizeof(MeshDrawCmdData));
	}
}

void OpenGlApi::RegisterCamera(Camera* camera) {
	if (camera != nullptr) {
		currentActiveCamera = camera;

		cascadeDataArray.clear();
		cascadeDataArray = std::vector<CascadeData>(cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.cascade_count", 4));

		for (size_t i = 0; i < cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.cascade_count", 4); i++) {
			
			const float fractionNear = static_cast<float>(i) / static_cast<float>(cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.cascade_count", 4));
			const float fractionFar = static_cast<float>(i + 1) / static_cast<float>(cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.cascade_count", 4));

			const float cameraClipRange = camera->farPlane - camera->nearPlane;
			const float cameraClipRatio = camera->farPlane / camera->nearPlane;

			const float uniformNear = camera->nearPlane + cameraClipRange * fractionNear;
			const float uniformFar = camera->nearPlane + cameraClipRange * fractionFar;

			const float logNear = camera->nearPlane * std::pow(cameraClipRatio, fractionNear);
			const float logFar = camera->nearPlane * std::pow(cameraClipRatio, fractionFar);

			const float blend = 0.1f;
			cascadeDataArray[i].nearPlane = blend * uniformNear + (1.f - blend) * logNear;
			cascadeDataArray[i].farPlane = blend * uniformFar + (1.f - blend) * logFar;
		}

		cascadeDataArray[0].nearPlane = camera->nearPlane;
		cascadeDataArray[cfg::Rendering.Read<size_t>("OpenGL", "opengl.shadows.cascade_count", 4) - 1].farPlane = camera->farPlane;
	}
}

void OpenGlApi::ShadowMapPass() {
	if (directionalLight == nullptr) return;

	for (size_t i = 0; i < cfg::Rendering.Read<uint32_t>("OpenGL", "opengl.shadows.cascade_count", 4); i++) {
		const float cascadeNearPlane = cascadeDataArray[i].nearPlane;
		const float cascadeFarPlane = cascadeDataArray[i].farPlane;

		const std::vector<Vec4> corners = GetFrustumCornersWorldSpace(currentActiveCamera->fov, 
																		   currentActiveCamera->aspectRatio, 
																		   cascadeNearPlane, 
																		   cascadeFarPlane, 
																		   currentActiveCamera->viewMatrix);
		const Mat4 view = GetLightViewMatrix(directionalLight->direction, corners);
		const Mat4 lightSpaceMatrix = GetLightSpaceMatrix(view, corners);
		cascadeDataArray[i].lightSpaceMatrix = lightSpaceMatrix;
	}

	cascadeDataUBO.UpdateData(0, cascadeDataArray.size() * sizeof(CascadeData), cascadeDataArray.data());

	BIND(Shader, shadowMapShader);
	BIND(FrameBuffer, shadowMapFBO);
	SetViewport(0, 0, shadowMapFBO.GetWidth(), shadowMapFBO.GetHeight());
	DrawScene(true, GL_FRONT);
}

void OpenGlApi::LightingPass() {
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
	lightingShader.SetUInt("cascadeCount", static_cast<uint32_t>(cascadeDataArray.size()));

	SetViewport(0, 0, lightingFBO.GetWidth(), lightingFBO.GetHeight());
	DrawScreenQuad(diffuseTex2D);
}

void OpenGlApi::OutputToScreen(const SceneTexture st) {
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

	BIND_TEX(Texture2D, sceneTexture, 0);
	BIND(Shader, screenShader);
	DrawScreenQuad(sceneTexture);
}

void OpenGlApi::DrawScreenQuad(Texture2D& texture) {
	BIND(VertexArray, screenQuadVAO);
	ClearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SetDepthEnabled(false);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void OpenGlApi::DrawScene(const bool withDepth, const GLenum faceCulling, const bool clear) {
	BIND(DrawIndirectBuffer, meshDIB);
	BIND(VertexArray, meshVAO);

	if (clear) {
		ClearBuffers(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		// ClearColor({0, 0, 0, 0});
	}
	
	SetDepthEnabled(withDepth);
	SetFaceCullMode(faceCulling);
	glMultiDrawElementsIndirect(GL_TRIANGLES,
								GL_UNSIGNED_INT,
								nullptr,
								static_cast<GLsizei>(meshDrawCmdDataArray.size()),
								sizeof(MeshDrawCmdData));
}

void OpenGlApi::PostFxPass()
{
	BIND_TEX(Texture2D, diffuseTex2D, 0);
	BIND(Shader, postFxShader);
	BIND(FrameBuffer, postfxFBO);
	postFxShader.SetUFloat("uGamma", cfg::Rendering.Read<float>("Video", "video.gamma", 2.2f));
	SetViewport(0, 0, postfxFBO.GetWidth(), postfxFBO.GetHeight());
	DrawScreenQuad(postFXTex2D);
}

void OpenGlApi::UpsertMeshEntity(const Entity entity, const StaticMeshRenderer* smRenderer, const Transform* transform) {
	Mat4 model = smRenderer->modelMatrix;
	Mat4 inverseModel = smRenderer->inverseModelMatrix;

	for (const Asset meshID : smRenderer->meshes) {
		Mesh* mesh = AssetManager::Get().GetAssetById<Mesh>(meshID);
		auto meshDataIt = assToMesh.find(meshID);
		if (assToMesh.find(meshID) == assToMesh.end()) {
			LOG(LogOpenGL, LOG_ERROR, std::format("Missing open GL mesh data for mesh {}.", mesh->assetPath));
			continue;
		}

		auto& meshInstances = assToMeshInsts[mesh->assetId];
		auto [entInstIt, inserted] = assToEntMeshInstIndexes[mesh->assetId].try_emplace(entity, static_cast<uint32_t>(meshInstances.size()));

		if (inserted) {
			meshInstances.emplace_back();
		}

		auto& instance = meshInstances[entInstIt->second];
		instance.model = model;
		instance.inverseModel = inverseModel;

		bool _isOnFrustum = IsOnFrustum(transform);
		instance.shouldDraw = _isOnFrustum;
	}
}

void OpenGlApi::RegisterPointLight(const Entity entity, const PointLight* light, const Transform* transform) {
	if (light == nullptr || transform == nullptr) {
		LOG(LogOpenGL, LOG_WARNING, "Failed to register point light. Light is null.");
		return;
	}

	auto [it, inserted] = entToPointLightData.try_emplace(entity);

	it->second.mPosition = transform->position;
	it->second.mColor = light->color;
	it->second.mIntensity = light->intensity;
	it->second.shadowFarPlane = light->shadowMapFarPlane;
	it->second.shadowNearPlane = light->shadowMapNearPlane;
	for (size_t i = 0; i < 6; i++)
		it->second.cubemapViewMatrices[i] = light->cubemapMatrices[i];

	if (inserted) {
		it->second.dataArrayIndex = static_cast<GLuint>(pointLightDataArray.size());
		pointLightDataArray.push_back(it->second);
	}

	pointLightDataArraySSBO.UpdateData(it->second.dataArrayIndex * sizeof(PointLightData), sizeof(PointLightData), &it->second);
}

void OpenGlApi::RegisterMesh(const Mesh* mesh) {
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

void OpenGlApi::UploadMeshRenderData() {
	std::vector<MeshInstanceData> meshInstDataArray;
	uint32_t vertices = 0;
	for (const auto& it : assToMeshInsts) {
		MeshMetaData& metaData = assToMesh[it.first];
		MeshDrawCmdData& cmdData = meshDrawCmdDataArray[metaData.drawCmdIndex];
		cmdData.baseInstance = static_cast<GLuint>(meshInstDataArray.size());

		const std::vector<MeshInstanceData>& meshInsts = it.second;

		// frustum culling
		for (auto& instanceData : it.second) {
			if (instanceData.shouldDraw) {
				meshInstDataArray.push_back(instanceData);
				vertices += cmdData.count / 3;
			}
		}
		cmdData.instanceCount = meshInstDataArray.size() - cmdData.baseInstance;
	}
	std::cout << "Vertices: " << vertices << std::endl;

	meshDIB.UpdateData(0, meshDrawCmdDataArray.size() * sizeof(MeshDrawCmdData), meshDrawCmdDataArray.data());
	meshSSBO.UpdateData(0, meshInstDataArray.size() * sizeof(MeshInstanceData), meshInstDataArray.data());
}

void OpenGlApi::GeometryPass() {
	BIND(Shader, gBufferShader);
	BIND(FrameBuffer, gBufferFBO);
	SetViewport(0, 0, gBufferFBO.GetWidth(), gBufferFBO.GetHeight());
	DrawScene(true, GL_BACK);
}

void OpenGlApi::SSAOPass() {
	SetViewport(0, 
				0, 
				cfg::Rendering.Read<uint32_t>("Video", "video.resX", 800), 
			    cfg::Rendering.Read<uint32_t>("Video", "video.resY", 600));

	{
		BIND_TEX(Texture2D, gViewPosition, 0);
		BIND_TEX(Texture2D, gViewNormal, 1);
		BIND_TEX(Texture2D, SSAONoiseTex2D, 2);
		BIND(Shader, ssaoShader);
		BIND(FrameBuffer, ssaoFBO);
		ClearColor({0, 0, 0, 1});
		DrawScreenQuad(SSAOTex2D);
	}

	{
		BIND_TEX(Texture2D, SSAOTex2D, 0);
		BIND(Shader, ssaoBlurShader);
		BIND(FrameBuffer, ssaoBlurFBO);
		ClearColor({0, 0, 0, 1});
		DrawScreenQuad(SSAOBlurTex2D);
	}

}

void OpenGlApi::SetDepthEnabled(const bool val)
{
	if (mDepthEnabled == val) return;
	
	val? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);        
	mDepthEnabled = val;
}

void OpenGlApi::SetFaceCullEnabled(const bool val)
{
	if (mFaceCullEnabled == val) return;
	
	val? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);        
	mFaceCullEnabled = val;
}

void OpenGlApi::SetFaceCullMode(const GLenum mode)
{
	if (mFaceCullMode == mode) return;
	glCullFace(mode);
}

void OpenGlApi::SetViewport(const GLint x_off, const GLint y_off, const GLsizei width, const GLsizei height)
{
	if (mViewport.x_offset == x_off &&
		mViewport.y_offset == y_off &&
		mViewport.width == width &&
		mViewport.height == height)
		return;

	mViewport.x_offset = x_off;
	mViewport.y_offset = y_off;
	mViewport.width = width;
	mViewport.height = height;

	glViewport(x_off, y_off, width, height);
}

void OpenGlApi::ClearColor(const Vec4 &rgba)
{
	glClearColor(rgba.r, rgba.g, rgba.b, rgba.a);
}

void OpenGlApi::ClearBuffers(const GLenum flags)
{
	glClear(flags);
}

GLenum OpenGlApi::internalToFormat(const GLenum internalFormat)
{
    switch (internalFormat) {
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
            return GL_RED;

        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
            return GL_RG;

        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16_SNORM:
        case GL_SRGB8:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
            return GL_RGB;

        case GL_RGBA2:
        case GL_RGBA4:
            return GL_RGB;

        // Map to GL_RGBA
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            return GL_RGBA;

        default:
            return 0;
    }
}
