#pragma once

#include <glm/glm.hpp>

struct PointLight {
	glm::vec3 position{ 0.0f };
	glm::vec3 color{ 1.0f };
	float intensity{ 1.0f };
	float radius{ 1.0f };
};