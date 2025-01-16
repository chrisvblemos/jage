#pragma once

#include <Core/Core.h>

struct RigidBody {
	glm::vec3 velocity{};
	glm::vec3 acceleration{};

	float mass = 1.0f; // kg
	float airResistance = 0.0f;
	float groundResistance = 0.0f;
	bool isKinematic = false;
	bool isGravityEnabled = true;
	float gravity = -9.81f;
};