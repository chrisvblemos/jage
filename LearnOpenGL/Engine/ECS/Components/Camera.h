#pragma once

#include <Engine/Core.h>

struct Camera {
	glm::vec3 position{ 0.0f };
	glm::quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
	float mMouseSensitivity = 0.1f;
	bool mIsOrthogonal = false;
	float mFOV = 60.0f;
	float mNearClipPlane = 0.1f;
	float mFarClipPlane = 1000.0f;
	float mAspectRatio = 4.0f / 3.0f;
	bool mIsActive = false;
	float pitch = 0, yaw = 0, roll = 0;

	glm::vec3 Forward() const {
		return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 Up() const {
		return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	glm::vec3 Right() const {
		return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
	}

	glm::mat4 ViewMatrix() const {
		return glm::lookAt(position, position + Forward(), Up());
	}

	glm::mat4 ProjectionMatrix() const {
		if (mIsOrthogonal)
			return { 1.0f };
		return glm::perspective(glm::radians(mFOV), mAspectRatio, mNearClipPlane, mFarClipPlane);
	}
};