#pragma once

#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <imgui/imgui.h>
#include <format>

#define LogGeneric "Generic"
#define LogAssetLoader "AssetLoader"
#define LogAssetManager "AssetManager"
#define LogSystemManager "SystemManager"
#define LogComponentManager "ComponentManager"
#define LogEntityManager "EntityManager"

#define LOG(module, level, message) Logging::Log(module, level, message)
#define LOG_INFO Logging::LogLevel::Info
#define LOG_DEBUG Logging::LogLevel::Debug
#define LOG_VERBOSE Logging::LogLevel::Verbose
#define LOG_WARNING Logging::LogLevel::Warning
#define LOG_ERROR Logging::LogLevel::Error
#define LOG_CRITICAL Logging::LogLevel::Critical

namespace Logging {
	enum LogLevel : uint8_t {
		Info,
		Debug,
		Verbose,
		Warning,
		Error,
		Critical,
		Count
	};

	inline std::string LogLevelToString(const LogLevel& level) {
		switch (level) {
		case LogLevel::Info: return "<Info>: ";
		case LogLevel::Debug: return "<Debug>: ";
		case LogLevel::Warning: return "<Warning>: ";
		case LogLevel::Error: return "<Error>: ";
		case LogLevel::Critical: return "<CRITICAL>: ";
		default: return "<Unknown>: ";
		};
	}

	inline void Log(const std::string& module, const LogLevel level, const std::string& message) {
		std::string res = module + LogLevelToString(level) + message;

		std::cout << res << std::endl;
		if (level == LogLevel::Critical)
			throw std::runtime_error(res);
	}

	// glm << overrides

	inline std::string vec2Str(const glm::vec2& vec) {
		return std::format("vec2({:.2f}, {:.2f})", vec.x, vec.y);
	}

	inline std::string vec3Str(const glm::vec3& vec) {
		return std::format("vec3({:.2f}, {:.2f}, {:.2f})", vec.x, vec.y, vec.z);
	}

	inline std::string quatStr(const glm::quat& quat) {
		return std::format("quat({:.2f}, {:.2f}, {:.2f}, {:.2f})", quat.x, quat.y, quat.z, quat.w);
	}
	
}