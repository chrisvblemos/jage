#pragma once

#include <stack>
#include <iostream>

//#include <glm/matrix.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <random>

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

	inline static void LogMatrix4(const glm::mat4& matrix, const std::string& name) {
		std::cout << name << ":\n";
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				std::cout << matrix[row][col] << " ";
			}
			std::cout << std::endl;
		}
	}

	inline static void LogVec3(const glm::vec3& v, const std::string& name) {
		std::cout << name << std::endl;
		std::cout << v.x << " " << v.y << " " << v.z << std::endl;
	}

	inline static void LogVec2(const glm::vec2& v, const std::string& name) {
		std::cout << name << std::endl;
		std::cout << v.x << " " << v.y << std::endl;
	}

	inline static glm::vec3 RandomPointInSphere(float radius) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		std::uniform_real_distribution<float> angleDist(0.0f, glm::two_pi<float>());

		float r = radius * std::cbrt(dist(gen));
		float theta = angleDist(gen);
		float phi = acos(1 - 2 * dist(gen));

		float x = r * sin(phi) * cos(theta);
		float y = r * sin(phi) * sin(theta);
		float z = r * cos(phi);

		return { x,y,z };

	}

	inline static float RandomFloat() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		return dist(gen);
	}

	inline static glm::quat RandomQuaternion() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		float u1 = dist(gen);
		float u2 = dist(gen);
		float u3 = dist(gen);

		float w = sqrt(1 - u1) * sin(glm::two_pi<float>() * u2);
		float x = sqrt(1 - u1) * cos(glm::two_pi<float>() * u2);
		float y = sqrt(u1) * sin(glm::two_pi<float>() * u3);
		float z = sqrt(u1) * cos(glm::two_pi<float>() * u3);

		return {w,x,y,z};
	}
}