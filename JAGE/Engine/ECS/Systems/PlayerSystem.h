#pragma once

#include <Core/Core.h>
#include "System.h"

class PlayerSystem : public System {
public:
	PlayerSystem() {
		name = "PlayerSystem";
	}

	void Update(float dt) override;
};