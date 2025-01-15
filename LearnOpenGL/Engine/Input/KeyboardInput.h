#pragma once

#include <Engine/Core.h>

#define KEY_W GLFW_KEY_W;
#define KEY_S GLFW_KEY_S;
#define KEY_A GLFW_KEY_A;
#define KEY_D GLFW_KEY_D;
#define KEY_SPACE GLFW_KEY_SPACE;

enum class EKeyState {
	Released,
	Pressed,
	Held,
};

enum EKey : int {
	None = 0,
	W = GLFW_KEY_W,
	S = GLFW_KEY_S,
	A = GLFW_KEY_A,
	D = GLFW_KEY_D,
	Space = GLFW_KEY_SPACE
};

struct GLFWwindow;

class KeyboardInput {
private:
	KeyboardInput() = default;
	GLFWwindow* window;
	std::unordered_map<EKey, EKeyState> keyStates;

public:
	// prevents copying
	KeyboardInput(const KeyboardInput&) = delete;
	KeyboardInput& operator=(const KeyboardInput&) = delete;

	static KeyboardInput& Get() {
		static KeyboardInput instance;
		return instance;
	}

	bool Initialize(GLFWwindow* window);
	void HandleInput();
	void RegisterKey(EKey key);
	bool IsKeyPressed(EKey key);
	bool IsKeyHeld(EKey key);
	bool IsKeyReleased(EKey key);

};