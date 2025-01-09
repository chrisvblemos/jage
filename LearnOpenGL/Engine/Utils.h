#include <stack>
#include <iostream>

//#include <glm/matrix.hpp>
#include <glm/glm.hpp>

namespace Utils {
	static uint32_t AllocateIdFromPool(std::stack<uint32_t>& pool, uint32_t& count) {
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

	static void LogMatrix4(const glm::mat4& matrix, const std::string& name) {
		std::cout << name << ":\n";
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				std::cout << matrix[row][col] << " ";
			}
			std::cout << std::endl;
		}
	}

	static void LogVec3(const glm::vec3& v, const std::string& name) {
		std::cout << name << std::endl;
		std::cout << v.x << " " << v.y << " " << v.z << std::endl;
	}

	static void LogVec2(const glm::vec2& v, const std::string& name) {
		std::cout << name << std::endl;
		std::cout << v.x << " " << v.y << std::endl;
	}
}