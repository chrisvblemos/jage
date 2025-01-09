#pragma once

#include <glm/glm.hpp>

struct PlayerMovement {
	glm::vec3 velocity{ 0.0f };
	float gravity = -9.81f;
	float movementSpeed = 5.0f;
	bool isGrounded = false;
	float pitch = 0.0f, yaw = 0.0f, roll = 0.0f;
};