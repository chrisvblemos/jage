#pragma once

#include <Core/Core.h>

struct RigidBody {
	Vec3 velocity = Vec3(0.0f);
	Vec3 acceleration = Vec3(0.0f);

	float mass = 1.0f; // kg
	float airResistance = 0.0f;
	float groundResistance = 0.0f;
	bool isKinematic = false;
	bool isGravityEnabled = true;
	float gravity = -9.81f;
};