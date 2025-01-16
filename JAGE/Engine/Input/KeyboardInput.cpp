#include "KeyboardInput.h"

bool KeyboardInput::Initialize(GLFWwindow* window) {
	if (window) {
		this->window = window;

		keyStates[EKey::W] = EKeyState::Released;
		keyStates[EKey::S] = EKeyState::Released;
		keyStates[EKey::A] = EKeyState::Released;
		keyStates[EKey::D] = EKeyState::Released;
		keyStates[EKey::Space] = EKeyState::Released;

		return true;
	}
	else {
		std::cout << "Failed to initialize KeyboardInput. Window is null." << std::endl;
		return false;
	}
}

void KeyboardInput::HandleInput() {
	if (!window) {
		return;
	}

	for (auto& keyState : keyStates) {
		if (glfwGetKey(window, static_cast<int>(keyState.first)) == GLFW_PRESS) {
			if (keyState.second == EKeyState::Released) {
				keyState.second = EKeyState::Pressed;
			}
			else {
				keyState.second = EKeyState::Held;
			}
		}
		else {
			if (keyState.second == EKeyState::Pressed || keyState.second == EKeyState::Held) {
				keyState.second = EKeyState::Released;
			}
		}
	}
}

bool KeyboardInput::IsKeyPressed(EKey key) {
	return keyStates[key] == EKeyState::Pressed;
}

bool KeyboardInput::IsKeyHeld(EKey key) {
	return keyStates[key] == EKeyState::Held;
}

bool KeyboardInput::IsKeyReleased(EKey key) {
	return keyStates[key] == EKeyState::Released;
}