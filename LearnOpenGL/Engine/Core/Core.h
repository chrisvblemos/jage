#pragma once

#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <stack>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <cassert>
#include <bitset>
#include <array>
#include <queue>

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>

#include <imgui/imgui.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace Constants {
	constexpr uint32_t MAX_ENTITIES = 5000;
	constexpr uint32_t MAX_COMPONENTS = 32;

	constexpr glm::vec3 NULL_VECTOR3 = glm::vec3(0.0f);
	constexpr glm::vec2 NULL_VECTOR2 = glm::vec2(0.0f);

	constexpr glm::mat3 IDENTITY_MAT3 = glm::mat3(1.0f);
	constexpr glm::mat4 IDENTITY_MAT4 = glm::mat4(1.0f);

	constexpr glm::vec3 WORLD_ORIGIN = glm::vec3(0.0f);
	constexpr glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 WORLD_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 WORLD_FORWARD = glm::vec3(0.0f, 0.0f, -1.0f);
}


using EntityId = int32_t;
using AssetId = int32_t;
using Entity = uint32_t;
using ComponentType = uint8_t;
using Signature = std::bitset<Constants::MAX_COMPONENTS>;

using Vector3 = glm::vec3;
using Vector2 = glm::vec2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;






