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
#include <format>
#include <type_traits>
#include <regex>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>

using Asset = int32_t;
using Entity = int32_t;
using ComponentType = uint8_t;

inline constexpr const Entity MAX_ENTITIES = 10000;
inline constexpr const ComponentType MAX_COMPONENT_TYPES = 32;
inline constexpr const Entity NULL_ENTITY = -1;
inline constexpr const Asset NULL_ASSET = -1;

using Signature = std::bitset<MAX_COMPONENT_TYPES>;
inline constexpr const Signature NO_SIGNATURE = 0;

using Quat = glm::quat;
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat2 = glm::mat2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

inline constexpr const Vec2 NullVec2 = Vec2(0.0f);
inline constexpr const Vec3 NullVec3 = Vec3(0.0f);
inline constexpr const Vec4 NullVec4 = Vec4(0.0f);
inline constexpr const Mat2 Id2 = Mat2(1.0f);
inline constexpr const Mat3 Id3 = Mat3(1.0f);
inline constexpr const Mat4 Id4 = Mat4(1.0f);
inline constexpr const Vec3 WOrigin = Vec3(0.0f);
inline constexpr const Vec3 WRight = Vec3(1.0f, 0.0f, 0.0f);
inline constexpr const Vec3 WForward = Vec3(0.0f, 0.0f, 1.0f);
inline constexpr const Vec3 WUp = Vec3(0.0f, 1.0f, 0.0f);