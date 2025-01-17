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