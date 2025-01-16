#pragma once

#include <Core/Core.h>
#include "System.h"

class PhysicsSystem : public System {
public:
	PhysicsSystem() = default;
	std::set<Entity> mEntities{};

	void Update(float dt);
};