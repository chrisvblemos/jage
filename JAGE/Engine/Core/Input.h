#pragma once

#include <Core/Core.h>

enum class KeyState {
	None,
	Pressed,
	Held,
	Released,
	MAX_KEY_STATES
};

class Input {
public:
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	static Input& Get() {
		static Input instance;
		return instance;
	}

	bool Initialize();

	Vec2 GetMouseDelta() const { return mouseDelta; }
	Vec2 GetMousePosition() const { return mousePos; }

	bool GetKeyWasPressed(const uint32_t key);
	bool GetKeyIsHeld(const uint32_t key);
	bool GetKeyIsReleased(const uint32_t key);

	void PollMouse();
	void PollKeys();

private:
	Input() = default;
	GLFWwindow* window;

	bool mouseFirstEvent = false;
	Vec2 mouseDelta;
	Vec2 mousePos;
	Vec2 mouseLastPos;

	std::array<KeyState, GLFW_KEY_LAST> keyStates;
};