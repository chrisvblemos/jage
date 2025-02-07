#pragma once

#include <Core/Core.h>

struct Character {
	Entity camera;
	bool cameraControlsYaw = true;
	float camMaxPitch = 89.0f;
	float camMinPitch = -89.0f;
	Vec3 velocity = Vec3(0.0f);
	float gravity = -9.81f;
	float maxMovementSpeed = 5.0f;
	bool isGrounded = false;
};