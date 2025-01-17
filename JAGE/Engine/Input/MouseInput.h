#pragma once

#include <Core/Core.h>

struct GLFWwindow;

class MouseInput {
private:
	MouseInput() = default;

	bool firstMouse = true;
	double lastX = 400, lastY = 300;
	glm::dvec2 mouseDelta = glm::dvec2(0);
	GLFWwindow* window;

public:
	// prevents copying
	MouseInput(const MouseInput&) = delete;
	MouseInput& operator=(const MouseInput&) = delete;

	static MouseInput& Get() {
		static MouseInput instance;
		return instance;
	}

	bool Initialize(GLFWwindow* window);
	void HandleInput();
	void SetMouseCursorVisibility(const bool value);

	inline glm::vec2 GetMouseDelta(double sensitivity = 1.0) { 
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			return mouseDelta * sensitivity;
		}
		
		return glm::vec2(0.0f);
	}
};