#pragma once

#include <glm/glm.hpp>

struct PointLight {
	glm::vec3 position{ 0.0f };
	glm::vec3 color{ 1.0f };
	float intensity{ 1.0f };
	float radius{ 1.0f };

	PointLight(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, float radius = 1.0f)
		: position(position), color(color), intensity(intensity), radius(radius) {
	};
};