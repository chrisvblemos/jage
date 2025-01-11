#include "Shaders.h"
#include "../World.h"
#include "../AssetManager.h"
#include "../Texture.h"
#include "../Material.h"
#include "../StaticMesh.h"
#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/DirectionalLight.h"
#include "../Components/PointLight.h"

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#include "OpenGL.h"

bool OpenGL::Initialize() {
	
	CompileShader(Shaders::LitDeferred);
	CompileShader(Shaders::UnlitDeferred);
	CompileShader(Shaders::Blit);
	CompileShader(Shaders::GBuffer);
	CompileShader(Shaders::ShadowMap);

	InitCameraUniformBufferObject();
	InitLightsUniformBufferObject();
	InitShadowMap();
	InitGBuffer();
	BufferScreenQuad();


	glEnable(GL_CULL_FACE);

	// all good
	return true;
}

void OpenGL::InitCameraUniformBufferObject() {
	glGenBuffers(1, &cameraUniformBufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBufferObject);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUniformBlock), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(
		GL_UNIFORM_BUFFER, 
		CAMERA_UNIFORM_BUFFER_INDEX, 
		cameraUniformBufferObject, 
		0, 
		sizeof(CameraUniformBlock)
	);
}

void OpenGL::InitLightsUniformBufferObject() {
	glGenBuffers(1, &lightsUniformBufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUniformBufferObject);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightsUniformBlock), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(
		GL_UNIFORM_BUFFER,
		LIGHTS_UNIFORM_BUFFER_INDEX,
		lightsUniformBufferObject,
		0,
		sizeof(LightsUniformBlock)
	);
}

void OpenGL::InitShadowMap() {
	glGenFramebuffers(1, &shadowMapFrameBufferObject);

	glGenTextures(1, &shadowMapTextureId);
	glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFrameBufferObject);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTextureId, 0);
	glDrawBuffer(GL_NONE); // color not needed for light depth map
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Renderer: Shadow map frame buffer is not complete!" << std::endl;
	}
	else {
		std::cout << "OpenGLApi: Shadow map frame buffer initialized." << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL::InitGBuffer() {
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	glGenRenderbuffers(1, &gDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Renderer: G-buffer frame buffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	std::cout << "Renderer: GBuffer frame buffer initialized." << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL::BufferTexture(Texture* texture) {
	if (texture == nullptr) {
		std::cout << "Renderer: Failed to buffer texture. Texture is null." << std::endl;
		return;
	}

	if (loadedTexturesTable.count(texture->assetId)) { // texture already loaded
		return;
	}

	GLuint textureId;
	GLenum format;

	if (texture->nrChannels == 1)
		format = GL_RED;
	else if (texture->nrChannels == 3)
		format = GL_RGB;
	else if (texture->nrChannels == 4)
		format = GL_RGBA;

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	loadedTexturesTable.emplace(texture->assetId, textureId);

	glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, texture->data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGL::BufferScreenQuad() {
	glGenVertexArrays(1, &screenQuadVertexArrayObject);
	glGenBuffers(1, &screenQuadVertexBufferObject);

	glBindVertexArray(screenQuadVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, screenQuadVertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	std::cout << "Renderer: Screen quad created." << std::endl;
}

void OpenGL::UploadLightUniforms() {
	LightsUniformBlock lightsUniformBlockData = {};
	lightsUniformBlockData.mAmbientLightColorAndIntensity = glm::vec4(glm::vec3(1.0f), 0.2f); // ambient color by hard-coded default
	
	// directional lights
	if (dirLight != nullptr) {
		DirectionalLightUniformData dirLightUData = {};
		dirLightUData.mDirection = glm::vec4(dirLight->direction, 1.0f);
		dirLightUData.mColorAndIntensity = glm::vec4(dirLight->color, dirLight->intensity);

		lightsUniformBlockData.mDirectionalLight = dirLightUData;
	}

	// point lights
	uint32_t pointLightsCount = static_cast<uint32_t>(std::clamp(pointLights.size(), size_t(0), size_t(OPENGL_MAX_POINT_LIGHTS)));
	if (pointLightsCount > 0) {
		PointLightUniformData pointLightUDatas[OPENGL_MAX_POINT_LIGHTS];
		for (unsigned int i = 0; i < pointLightsCount; i++) {
			PointLight* pointLight = pointLights[i];

			if (pointLight != nullptr) {
				PointLightUniformData pointLightUData = {};
				pointLightUData.mColorAndIntensity = glm::vec4(pointLight->color, pointLight->intensity);
				pointLightUData.mPositionAndRadius = glm::vec4(pointLight->position, pointLight->radius);

				lightsUniformBlockData.mPointLights[i] = pointLightUData;
			}
		}
	}
	lightsUniformBlockData.mPointLightsCount = pointLightsCount;

	glBindBuffer(GL_UNIFORM_BUFFER, lightsUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightsUniformBlock), &lightsUniformBlockData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	pointLights.clear();
	dirLight = nullptr;
}

void OpenGL::UploadCameraUniforms(const Camera* camera) {
	if (camera == nullptr) {
		return;
	}

	CameraUniformBlock cameraUniformBlockData = {};
	cameraUniformBlockData.mPosition = glm::vec4(camera->position, 1.0f);
	cameraUniformBlockData.mProjMat = camera->GetProjectionMatrix();
	cameraUniformBlockData.mViewMat = camera->GetViewMatrix();

	glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniformBlock), &cameraUniformBlockData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGL::ShadowMapPass() {
	if (dirLight == nullptr) {
		return;
	}

	glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFrameBufferObject);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	Shader& shader = Shaders::ShadowMap;
	UseShader(&shader);
	SetShaderMat4(&shader, "uLightSpaceMatrix", dirLight->GetLightSpaceTMatrix());

	// we have to draw the scene from the directional
	// light's perspective
	glCullFace(GL_FRONT);

	for (const auto& it : meshVertexArrayObjToInstanceIdMap) {
		uint32_t vao = it.first;
		std::vector<Entity> instances = it.second;
		StaticMesh* staticMesh = meshVertexArrayObjToStaticMeshMap[vao];

		if (vao == 0 || staticMesh == nullptr || instances.empty()) {
			continue;
		}

		glBindVertexArray(vao);

		for (Entity instance: instances) {
			Transform* transform = instanceIdToTransformMap[instance];
			if (transform == nullptr) {
				continue;
			}

			glm::mat4 modelMatrix = CalculateModelMatrix(transform->position, transform->rotation, transform->scale);
			SetShaderMat4(&shader, "uModel", modelMatrix);

			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(staticMesh->indices.size()),
				GL_UNSIGNED_INT,
				nullptr
			);
		}
	}

	glBindVertexArray(0);
	glCullFace(GL_BACK);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL::LightPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader& shader = Shaders::LitDeferred;
	UseShader(&shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);

	SetShaderInt(&shader, "gPosition", 0);
	SetShaderInt(&shader, "gNormal", 1);
	SetShaderInt(&shader, "gAlbedoSpec", 2);
	SetShaderInt(&shader, "uShadowMap", 3);

	if (dirLight != nullptr) {
		glm::mat4 lightSpaceMatrix = dirLight->GetLightSpaceTMatrix();
		SetShaderMat4(&shader, "uLightSpaceMatrix", dirLight->GetLightSpaceTMatrix());
	}

	UploadLightUniforms();
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(screenQuadVertexArrayObject);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void OpenGL::DebugGbuffer(uint32_t layer) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	if (layer == 0) {
		glBindTexture(GL_TEXTURE_2D, gPosition);
	}
	if (layer == 1) {
		glBindTexture(GL_TEXTURE_2D, gNormal);
	}
	if (layer == 2) {
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	}
	if (layer == 3) {
		glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);
	}

	Shader& shader = Shaders::Blit;
	UseShader(&shader);
	SetShaderInt(&shader, "screenTexture", 0);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(screenQuadVertexArrayObject);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void OpenGL::BufferStaticMesh(Entity instanceId, StaticMesh* staticMesh, Transform* transform) {
	if (transform == nullptr || staticMesh == nullptr) {
		return;
	}

	if (staticMesh == nullptr || staticMesh->VAO == 0 || transform == nullptr) {
		glGenVertexArrays(1, &staticMesh->VAO);
		glGenBuffers(1, &staticMesh->VBO);
		glGenBuffers(1, &staticMesh->EBO);

		glBindVertexArray(staticMesh->VAO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticMesh->EBO); // store indices
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			staticMesh->indices.size() * sizeof(uint32_t),
			staticMesh->indices.data(),
			GL_STATIC_DRAW
		);

		glBindBuffer(GL_ARRAY_BUFFER, staticMesh->VBO);
		glBufferData(
			GL_ARRAY_BUFFER,
			staticMesh->vertices.size() * sizeof(Vertex),
			staticMesh->vertices.data(),
			GL_STATIC_DRAW
		);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
		glEnableVertexAttribArray(0); // vertices
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(1); // normals
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		glEnableVertexAttribArray(2); // text coords
	
		glBindVertexArray(0);

		// store data
		// vao->ids map
		// vao->meshes map
		meshVertexArrayObjToStaticMeshMap[staticMesh->VAO] = staticMesh;
	}

	// entity->transform map
	if (instanceIdToTransformMap.find(instanceId) == instanceIdToTransformMap.end()) {
		meshVertexArrayObjToInstanceIdMap[staticMesh->VAO].push_back(instanceId);
		instanceIdToTransformMap[instanceId] = transform;
	}
}

glm::mat4 OpenGL::CalculateModelMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::mat4_cast(rotation) * modelMatrix;
	modelMatrix = glm::scale(modelMatrix, scale);
	return modelMatrix;
}

void OpenGL::GeometryPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	for (const auto& it : meshVertexArrayObjToInstanceIdMap) {
		uint32_t vao = it.first;
		std::vector<Entity> instances = it.second;
		StaticMesh* staticMesh = meshVertexArrayObjToStaticMeshMap[vao];

		if (vao == 0 || staticMesh == nullptr || instances.size() == 0) {
			return;
		}

		Shader& shader = Shaders::GBuffer;
		UseShader(&shader);

		// load textures from material, if there are any
		// if not, just fall back to default
		// solid color (white, 0.5 specular)
		if (Material* material = staticMesh->material) {
			SetShaderVec3(&shader, "uBaseColor", material->mBaseColor);
			SetShaderBool(&shader, "uHasDiffuseTexture", material->HasDiffuseTexture());
			if (Texture* diffuseTexture = material->diffuseTexture) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, loadedTexturesTable[diffuseTexture->assetId]);
				SetShaderInt(&shader, "uDiffuseTexture", 0);
			}

			SetShaderFloat(&shader, "uBaseSpecular", material->mBaseSpecular);
			SetShaderBool(&shader, "uHasSpecularTexture", material->HasSpecularTexture());

			if (Texture* specularTexture = material->specularTexture) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, loadedTexturesTable[specularTexture->assetId]);
				SetShaderInt(&shader, "uSpecularTexture", 1);
			}
		}

		// render instances
		glBindVertexArray(vao);
		for (const Entity instance : instances) {
			Transform* transform = instanceIdToTransformMap[instance];
			if (transform == nullptr) {
				continue;
			}

			glm::mat4 modelMatrix = CalculateModelMatrix(transform->position, transform->rotation, transform->scale);
			glm::mat4 modelInvMatrixT = glm::inverseTranspose(modelMatrix);

			
			SetShaderMat4(&shader, "model", modelMatrix);
			SetShaderMat4(&shader, "invModelT", modelInvMatrixT);

			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(staticMesh->indices.size()),
				GL_UNSIGNED_INT,
				nullptr
			);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
}

void OpenGL::CompileShader(Shader& shader) {
	std::string vertexCode = shader.GetVertexCode();
	std::string fragmentCode = shader.GetFragmentCode();

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	uint32_t vertex, fragment;
	int success;
	char infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, nullptr);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		std::cout << "OpenGLApi: Vertex code compilation failed for shader " << shader.assetName << " Info: \n" << infoLog << std::endl;
		return;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, nullptr);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
		std::cout << "OpenGLApi: Fragment code compilation failed for shader " << shader.assetName << " Info: \n" << infoLog << std::endl;
		return;
	};

	uint32_t ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, nullptr, infoLog);
		std::cout << "OpenGLApi: Program link failed for shader " << shader.assetName << " Info: \n" << infoLog << std::endl;
		return;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	shader.isCompiled = true;
	shader.id = ID;
}

void OpenGL::UseShader(Shader* shader) {
	glUseProgram(shader->id);
}

void OpenGL::SetShaderBool(Shader* shader, const std::string& name, const bool value) {
	if (!shader) {
		std::cout << "Shader is null." << std::endl;
		return;
	}

	if (shader->isCompiled) {
		glUniform1i(glGetUniformLocation(shader->id, name.c_str()), (int)value);
		return;
	}

	std::cout << "Shader " << shader->assetName << " not compiled for setting bool." << std::endl;
}


void OpenGL::SetShaderInt(Shader* shader, const std::string& name, const int value) {
	if (!shader) {
		std::cout << "Shader is null." << std::endl;
		return;
	}

	if (shader->isCompiled) {
		glUniform1i(glGetUniformLocation(shader->id, name.c_str()), value);
		return;
	}
	
	std::cout << "Shader " << shader->assetName << " not compiled for setting int." << std::endl;
}

void OpenGL::SetShaderFloat(Shader* shader, const std::string& name, const float value) {
	if (!shader) {
		std::cout << "Shader is null." << std::endl;
		return;
	}

	if (shader->isCompiled) {
		glUniform1f(glGetUniformLocation(shader->id, name.c_str()), value);
		return;
	}

	std::cout << "Shader " << shader->assetName << " not compiled for setting float." << std::endl;
}

void OpenGL::SetShaderMat4(Shader* shader, const std::string& name, const glm::mat4& value) {
	if (!shader) {
		std::cout << "Shader is null." << std::endl;
		return;
	}

	if (shader->isCompiled) {
		glUniformMatrix4fv(glGetUniformLocation(shader->id, name.c_str()), 1, GL_FALSE, &value[0][0]);
		return;
	}

	std::cout << "Shader " << shader->assetName << " not compiled for setting mat4." << std::endl;
}

void OpenGL::SetShaderVec3(Shader* shader, const std::string& name, const glm::vec3& value) {
	if (!shader) {
		std::cout << "Shader is null." << std::endl;
		return;
	}

	if (shader->isCompiled) {
		glUniform3fv(glGetUniformLocation(shader->id, name.c_str()), 1, &value[0]);
		return;
	}

	std::cout << "Shader " << shader->assetName << " not compiled for setting vec3." << std::endl;
}

void OpenGL::SetShaderVec2(Shader* shader, const std::string& name, const glm::vec2& value) {
	if (!shader) {
		std::cout << "Shader is null." << std::endl;
		return;
	}

	if (shader->isCompiled) {
		glUniform2fv(glGetUniformLocation(shader->id, name.c_str()), 1, &value[0]);
		return;
	}

	std::cout << "Shader " << shader->assetName << " not compiled for setting vec2." << std::endl;
}