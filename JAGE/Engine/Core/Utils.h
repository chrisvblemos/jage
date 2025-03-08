#pragma once

#include <stack>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <random>
#include <Common.h>

namespace Utils {
	inline static uint32_t AllocateIdFromPool(std::stack<uint32_t>& pool, uint32_t& count) {
		uint32_t id;
		if (!pool.empty()) {
			id = pool.top();
			pool.pop();
		}
		else {
			id = count++;
		}

		return id;
	}

	inline static Vec3 RandomPointInSphere(const float radius, const Vec3& origin = Vec3(0.0f)) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		std::uniform_real_distribution<float> angleDist(0.0f, glm::two_pi<float>());

		float r = radius * std::cbrt(dist(gen));
		float theta = angleDist(gen);
		float phi = acos(1 - 2 * dist(gen));

		float x = origin.x + r * sin(phi) * cos(theta);
		float y = origin.y + r * sin(phi) * sin(theta);
		float z = origin.z + r * cos(phi);

		return { x,y,z };

	}

	inline static float RandomFloat() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		return dist(gen);
	}

	inline static Vec3 RandomEulerRotation() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		float u1 = dist(gen);
		float u2 = dist(gen);
		float u3 = dist(gen);

		float pitch = u1 * 360.0f;
		float yaw = u2 * 360.0f;
		float roll = u3 * 360.0f;

		return { pitch, yaw, roll };
	}

	inline static Vec3 ToDegrees(const Vec3& r) {
		return r * 180.0f / glm::pi<float>();
	}

	inline static float ToDegrees(const float a) {
		return a * 180.0f / glm::pi<float>();
	}

	inline static Vec3 ToRad(const Vec3& r) {
		return r * glm::pi<float>() / 180.0f;
	}

	inline static float ToRad(const float a) {
		return a * glm::pi<float>() / 180.0f;
	}

	inline static Quat EulerToQuat(const Vec3& euler) {
		return glm::quat(euler);
	}

	inline static Vec3 QuatToEuler(const Quat& quat) {
		return glm::eulerAngles(quat);
	}
}