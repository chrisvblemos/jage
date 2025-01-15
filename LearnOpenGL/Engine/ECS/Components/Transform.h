#pragma once

#include <Engine/Core.h>

#define PI glm::pi<float>()
#define VEC3_UNIT glm::vec3(1.0f)
#define VEC3_ORIGIN glm::vec3(0.0f)
#define VEC3_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define QUAT_NO_ROTATION glm::quat(glm::vec3(0.0, 0.0f, 0.0f))
#define QUAT_DOWN glm::quat(glm::vec3(PI / 2.0f, 0.0f, 0.0f))

struct Transform {
	Transform(glm::vec3 position = glm::vec3(0.0f), glm::quat rotation = glm::quat(glm::vec3(0.0f)), glm::vec3 scale = glm::vec3(1.0f))
		: position(position), rotation(rotation), scale(scale) {
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