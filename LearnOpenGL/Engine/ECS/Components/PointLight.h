#pragma once

#include <Engine/Core.h>

struct PointLight {
	glm::vec3 position{ 0.0f };
	glm::vec3 color{ 1.0f };
	float intensity{ 1.0f };
	float radius{ 1.0f };
	int glShadowMapIndex{ -1 };

	float shadowMapResolution{ 2048 };
	float shadowMapNearPlane{ 1.0f };
	float shadowMapFarPlane{ 25.0f };

	PointLight(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, float radius = 1.0f)
		: position(position), color(color), intensity(intensity), radius(radius) {
	};

	std::vector<glm::mat4> GetCubemapLightSpaceMatrices(glm::vec3 cameraPos) const {
		std::vector <glm::mat4> cubeMapMatrices;

		glm::mat4 projMatrix = GetProjMatrix();

		// generate 6 shadow maps, for each cube face
		cubeMapMatrices.push_back(projMatrix *
			glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		cubeMapMatrices.push_back(projMatrix *
			glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		cubeMapMatrices.push_back(projMatrix * 
			glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		cubeMapMatrices.push_back(projMatrix *
			glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		cubeMapMatrices.push_back(projMatrix *
			glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		cubeMapMatrices.push_back(projMatrix *
			glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

		return cubeMapMatrices;
	}

	glm::mat4 GetProjMatrix() const {
		return glm::perspective(glm::radians(90.0f), 1.0f, shadowMapNearPlane, shadowMapFarPlane);
	}
};