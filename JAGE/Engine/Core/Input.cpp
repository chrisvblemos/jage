#include "Input.h"

bool Input::Initialize()
{
	GLFWwindow* window = Window::Get().GetNativeWindow();
	if (!window) return false;

	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	this->window = window;
	return true;
}

bool Input::GetKeyWasPressed(const uint32_t key)
{
	return keyStates[key] == KeyState::Pressed;
}

bool Input::GetKeyIsHeld(const uint32_t key)
{
	return keyStates[key] == KeyState::Held;
}

bool Input::GetKeyIsReleased(const uint32_t key)
{
	return keyStates[key] == KeyState::Released;
}

void Input::PollMouse()
{
	if (!window) return;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	mousePos = { static_cast<float>(xpos), static_cast<float>(ypos) };
	if (mouseFirstEvent) {
		mouseLastPos = mousePos;
		mouseFirstEvent = false;
	}

	mouseDelta = mousePos - mouseLastPos;
	mouseLastPos = mousePos;
}

void Input::PollKeys()
{
	if (!window) return;

	for (size_t i = 0; i < GLFW_KEY_LAST; i++) {
		if (glfwGetKey(window, i) == GLFW_PRESS) {
			if (keyStates[i] == KeyState::Pressed) {
				keyStates[i] = KeyState::Held;
			}
			else {
				keyStates[i] = KeyState::Pressed;
			}
		}
		else {
			if (keyStates[i] == KeyState::Pressed || keyStates[i] == KeyState::Held) {
				keyStates[i] = KeyState::Released;
			}
		}
	}

#ifdef _DEBUG
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
#endif
}
