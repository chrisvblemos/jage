#pragma once

#include <Core/Core.h>
#include <Logging.h>

// Callback function for OpenGL debug messages
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam) {
	const char* sourceStr;
	const char* typeStr;
	const char* severityStr;

	// Determine the source of the message
	switch (source) {
	case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
	default:                              sourceStr = "Unknown"; break;
	}

	// Determine the type of the message
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability Issue"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance Issue"; break;
	case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
	default:                                typeStr = "Unknown"; break;
	}

	// Determine the severity of the message
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
	default:                             severityStr = "Unknown"; break;
	}

	std::string msg = std::format("Source: {}, Type: {}, ID: {}, Message: {}", sourceStr, type, id, message);

	// Print debug message
	//std::cerr << "[OpenGL Debug] Source: " << sourceStr
	//	<< ", Type: " << typeStr
	//	<< ", Severity: " << severityStr
	//	<< ", ID: " << id
	//	<< ", Message: " << message << std::endl;

	if (type == GL_DEBUG_TYPE_ERROR) {
		LOG(LogGeneric, LOG_ERROR, msg);
	}
	else {
		LOG(LogGeneric, LOG_VERBOSE, msg);
	}

#ifdef _DEBUG
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		__debugbreak(); // Break into debugger on high-severity errors (Visual Studio only)
	}
#endif
}

// Initialize OpenGL debug output
void EnableOpenGLDebugOutput() {
	glEnable(GL_DEBUG_OUTPUT); // Enable debug output
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Make it synchronous for easier debugging
	glDebugMessageCallback(GLDebugMessageCallback, nullptr); // Register callback function

	// Optionally control which messages to log (filtering)
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

namespace GLUtils {
	/* Projection matrix uses camera fov and aspect ratio, and the near and far plane
of the frustum in question. The view matrix is the camera's view matrix as usual. */
	std::vector<glm::vec4> GetFrustumCornersWorldSpace(const float fov, const float aspectRatio, const float nearPlane, const float farPlane, const glm::mat4& view) {
		const glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		const auto inv = glm::inverse(proj * view);

		std::vector<glm::vec4> corners;
		for (int x = 0; x < 2; x++) {
			for (int y = 0; y < 2; y++) {
				for (int z = 0; z < 2; z++) {
					const glm::vec4 point =
						inv * glm::vec4(
							2.0f * x - 1.0f,
							2.0f * y - 1.0f,
							2.0f * z - 1.0f,
							1.0f);
					corners.push_back(point / point.w);
				}
			}
		}

		return corners;
	}

	glm::mat4 GetLightViewMatrix(const glm::vec3& lightDir, const std::vector<glm::vec4>& frustumCorners) {
		glm::vec3 center = glm::vec3(0.0f); // center of light frustum
		for (const auto& v : frustumCorners) {
			center += glm::vec3(v);
		}

		center /= (float)frustumCorners.size();

		glm::vec3 normalizedLightDir = glm::normalize(lightDir);
		float pitch = glm::asin(normalizedLightDir.y);
		float yaw = glm::atan(normalizedLightDir.x, normalizedLightDir.z);

		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 translate = glm::translate(view, -(center - lightDir));
		glm::mat4 rotYaw = glm::rotate(view, -yaw, { 0.0f, 1.0f, 0.0f });
		glm::mat4 rotPitch = glm::rotate(view, -pitch, { 1.0f, 0.0f, 0.0f });
		return rotPitch * rotYaw * translate;
	}

	/* Returns the light proj matrix when using CSM. To get the view-proj matrix,
	just multiply by the view matrix (see GetLightViewMatrix()).*/
	glm::mat4 GetLightSpaceMatrix(const glm::mat4& lightViewMatrix, const std::vector<glm::vec4>& corners) {
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();

		for (const auto& v : corners)
		{
			const auto trf = lightViewMatrix * v;
			minX = glm::min(minX, trf.x);
			maxX = glm::max(maxX, trf.x);
			minY = glm::min(minY, trf.y);
			maxY = glm::max(maxY, trf.y);
			minZ = glm::min(minZ, trf.z);
			maxZ = glm::max(maxZ, trf.z);
		}

		//auto tmp = -minZ;
		//minZ = -maxZ;
		//maxZ = tmp;

		//auto mid = (maxZ - minZ) / 2;
		//minZ -= mid * 5.0f;
		//maxZ += mid * 5.0f;

		constexpr float zMult = 10.0f; // multiplier to extend light view frustum to 
		// corners of camera (objects out of view still cast shadows)
		if (minZ < 0)
			minZ *= zMult;
		else
			minZ /= zMult;

		if (maxZ < 0)
			maxZ /= zMult;
		else
			maxZ *= zMult;

		//auto mid = (maxZ - minZ) / 2.0;
		//minZ -= mid * 5.0f;
		//maxZ += mid * 5.0f;

		const glm::mat4 lightProjection = glm::ortho(
			minX,
			maxX,
			minY,
			maxY,
			minZ,
			maxZ
		);

		return lightProjection * lightViewMatrix;
	}
}


