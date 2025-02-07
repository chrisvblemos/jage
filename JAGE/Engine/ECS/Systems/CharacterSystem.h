#pragma once

#include "System.h"

class CharacterSystem : public System {
public:
	CharacterSystem() {
		name = "CharacterSystem";
	}

	void Update(float dt) override;
};