#pragma once

#include "System.h"

class MouseInput;
class KeyboardInput;

extern MouseInput gMouseInput;
extern KeyboardInput gKeyboardInput;

class PlayerSystem : public System {
public:
	PlayerSystem() = default;
	void Update(float dt);
};