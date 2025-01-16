#pragma once

#include <Core/Core.h>
#include "System.h"

class PlayerSystem : public System {
public:
	std::set<Entity> mEntities{};

	PlayerSystem() = default;
	void Update(float dt);
};