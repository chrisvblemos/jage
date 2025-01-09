#pragma once

#include "System.h"

class PhysicsSystem : public System {
public:
	PhysicsSystem() = default;

	void Update(float dt);
};