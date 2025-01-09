#pragma once

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>

struct Transform {
	Transform(glm::vec3 position = glm::vec3(0.0f))
		: position(position) {
	};

	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
	glm::vec3 scale = glm::vec3(1.0f);

	glm::vec3 Forward() {
		return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 Up() {
		return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	glm::vec3 Right() {
		return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
	}
};