#pragma once

#include <ECS/Components/Character.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>
#include "System.h"

class CharacterSystem : public System {
public:
	CharacterSystem() {
		name = "CharacterSystem";
	}

	void Update(float dt) override;
	void FixedUpdate(float dt) override;

private:
	Entity characterEntity = -1;
	Vec3 moveInput;
};