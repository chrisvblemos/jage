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

	std::array<glm::vec3, 8> GetFrustumCornersInViewSpace() {
		std::array < glm::vec3, 8> res;

		const std::array<glm::vec3, 8> ndcCorners = {
		glm::vec3(-1.0f, -1.0f, -1.0f), // Near bottom-left
		glm::vec3(1.0f, -1.0f, -1.0f), // Near bottom-right
		glm::vec3(-1.0f,  1.0f, -1.0f), // Near top-left
		glm::vec3(1.0f,  1.0f, -1.0f), // Near top-right
		glm::vec3(-1.0f, -1.0f,  1.0f), // Far bottom-left
		glm::vec3(1.0f, -1.0f,  1.0f), // Far bottom-right
		glm::vec3(-1.0f,  1.0f,  1.0f), // Far top-left
		glm::vec3(1.0f,  1.0f,  1.0f)  // Far top-right
		};

		const glm::mat4 invProjection = glm::inverse(ProjectionMatrix());
		for (int i = 0; i < 8; i++) {
			glm::vec4 corner = invProjection * glm::vec4(ndcCorners[i], 1.0f);
			corner /= corner.w;
			res[i] = glm::vec3(corner);
		}

		return res;
	}

	std::array<glm::vec3, 8> GetFrustumCornersInWorldSpace() {
		std::array<glm::vec3, 8> res;

		const glm::mat4 inverseView = glm::inverse(ViewMatrix());
		const std::array<glm::vec3, 8> frustumCornersViewSpace = GetFrustumCornersInViewSpace();
		for (int i = 0; i < 8; i++) {
			glm::vec4 corner = inverseView * glm::vec4(frustumCornersViewSpace[i], 1.0f);
			res[i] = glm::vec3(corner);
		}

		return res;
	}

	std::vector<float> GetCascadeSplits(const uint32_t numCascades) {
		std::vector<float> splits(numCascades);

		float clipRange = mFarClipPlane - mNearClipPlane;
		float ratio = mFarClipPlane / mNearClipPlane;

		for (int i = 0; i <= numCascades; i++) {
			float p = i / static_cast<float>(numCascades);
			float logSplit = mNearClipPlane * std::pow(ratio, p);
			float linearSplit = mNearClipPlane + clipRange * p;

			// practical split (mix linear and log splits)
			splits[i] = glm::mix(linearSplit, logSplit, 0.5f);
		}

		return splits;
	}
};