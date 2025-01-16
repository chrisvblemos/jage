#include "MouseInput.h"

bool MouseInput::Initialize(GLFWwindow* window) {
	if (window)
	{
		this->window = window;
		return true;
	}

	std::cout << "Failed to initialize MouseInput. Window is null." << std::endl;
	return false;
};


void MouseInput::HandleInput() {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	float xposf = static_cast<float>(xpos);
	float yposf = static_cast<float>(ypos);

	if (firstMouse) {
		lastX = xposf;
		lastY = yposf;
		firstMouse = false;
	}

	mouseDelta = { xposf - lastX, yposf - lastY };

	lastX = xposf;
	lastY = yposf;
};