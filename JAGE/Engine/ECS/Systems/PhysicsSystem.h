#pragma once

#include <Core/Core.h>
#include "System.h"

class PhysicsSystem : public System {
public:
	PhysicsSystem() {
		name = "PhysicsSystem";
	}

	void FixedUpdate(float dt) override;
};