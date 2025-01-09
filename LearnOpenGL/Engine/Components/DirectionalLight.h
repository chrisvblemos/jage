#pragma once

#include <glm/glm.hpp>

struct DirectionalLight {
public:
	glm::vec3 direction{ 1.0f };
	glm::vec3 color{ 1.0f };
	float intensity{ 1.0f };
};