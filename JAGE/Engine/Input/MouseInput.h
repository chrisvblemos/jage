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

	inline glm::vec2 GetMouseDelta(double sensitivity = 1.0) { return mouseDelta * sensitivity; }
};