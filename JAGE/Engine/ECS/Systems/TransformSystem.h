#pragma once

#include <Core/Core.h>
#include "System.h"

class TransformSystem : public System {
public:
	TransformSystem() {
		name = "TransformSystem";
	}

	void Update(float dt) override;
};
