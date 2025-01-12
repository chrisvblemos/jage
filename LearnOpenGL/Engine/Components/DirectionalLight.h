#pragma once

#include <glm/glm.hpp>

struct DirectionalLight {
public:
	glm::vec3 direction{ 1.0f };
	glm::vec3 color{ 1.0f };
	float intensity{ 1.0f };
	float shadowDistance = { 100.0f };

	uint32_t shadowMapResolution = 2048;
	float shadowMapNearPlane = 1.0f;
	float shadowMapFarPlane = 500.0f;
	glm::vec4 orthoProjSizes = glm::vec4(10.0f);

	glm::mat4 GetViewMatrix(glm::vec3 cameraPos) {
		glm::vec3 lightPos = cameraPos - direction * (shadowDistance * 0.5f);
		//glm::vec3 pos = 100.0f * glm::vec3(0.0f, 1.0f, 0.0f);
		return glm::lookAt(lightPos, cameraPos, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::mat4 GetProjMatrix() {
		return glm::ortho(
			-orthoProjSizes.x, 
			orthoProjSizes.y, 
			-orthoProjSizes.z, 
			orthoProjSizes.w, 
			shadowMapNearPlane, 
			shadowMapFarPlane
		);
	}

	glm::mat4 GetLightSpaceTMatrix(glm::vec3 cameraPos) {
		return GetProjMatrix() * GetViewMatrix(cameraPos);
	}


	DirectionalLight(glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f)
		: direction(direction), color(color), intensity(intensity) {
	};
};