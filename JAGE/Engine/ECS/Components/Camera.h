#pragma once

#include <Core/Core.h>


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
		float yawRadians = glm::radians(yaw);
		float pitchRadians = glm::radians(pitch);
		return { glm::sin(yawRadians) * glm::sin(pitchRadians), glm::cos(yawRadians) * glm::sin(pitchRadians), -glm::cos(pitchRadians) };
		//return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 Up() const {
		return glm::cross(Forward(), Right());
		//return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	glm::vec3 Right() const {
		float yawRadians = glm::radians(yaw);
		return { glm::cos(yawRadians), glm::sin(yawRadians), 0.0f };
		//return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
	}

	glm::mat4 ViewMatrix() const {
		float yawRadians = glm::radians(yaw);
		float pitchRadians = glm::radians(pitch);
		float rollRadians = glm::radians(roll);

		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 translate = glm::translate(view, -position);
		glm::mat4 rotYaw = glm::rotate(view, -yaw, {0.0f, 1.0f, 0.0f});
		glm::mat4 rotPitch = glm::rotate(view, -pitch, { 1.0f, 0.0f, 0.0f });
		glm::mat4 rotRoll = glm::rotate(view, -roll, { 0.0f, 0.0f, 1.0f });
		return rotRoll * rotPitch * rotYaw * translate * view;
		//return glm::lookAt(position, position + Forward(), Up());
	}

	glm::mat4 ProjectionMatrix() const {
		if (mIsOrthogonal)
			return { 1.0f };
		return glm::perspective(glm::radians(mFOV), mAspectRatio, mNearClipPlane, mFarClipPlane);
	}
};