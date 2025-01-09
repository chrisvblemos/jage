#include "Input/KeyboardInput.h"
#include "Input/MouseInput.h"
#include "Components/PlayerMovement.h"
#include "Components/Camera.h"
#include "Components/Transform.h"
#include "World.h"
#include "PlayerSystem.h"
#include "Utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <iostream>

const float MAX_CAMERA_PITCH = glm::radians(89.0f);
const float MIN_CAMERA_PITCH = glm::radians(-89.0f);

void PlayerSystem::Update(float dt) {
	for (const Entity& entity : mEntities)
	{
		Transform& transform = World::Get().GetComponent<Transform>(entity);
		Camera& camera = World::Get().GetComponent<Camera>(entity);
		PlayerMovement& playerMovement = World::Get().GetComponent<PlayerMovement>(entity);

		// camera control
		const glm::vec2 mouseDelta = MouseInput::Get().GetMouseDelta(camera.mMouseSensitivity);

		// the player entity only rotates in the yaw axis
		// camera pitch is stored inside the camera component
		if (mouseDelta.x) {
			camera.yaw -= mouseDelta.x * dt;
		}

		// clamp pitch between -89.0f and 89.0f
		if (mouseDelta.y) {
			camera.pitch -= mouseDelta.y * dt;
			camera.pitch = glm::clamp(camera.pitch, MIN_CAMERA_PITCH, MAX_CAMERA_PITCH);
		}

		transform.rotation = glm::quat(glm::vec3(playerMovement.pitch, camera.yaw, 0.0f));
		camera.rotation = glm::quat(glm::vec3(camera.pitch, camera.yaw, 0.0f));
		
		const glm::vec3 forward = transform.Forward();
		const glm::vec3 right = transform.Right();

		// player movement control
		glm::vec3 moveInput = glm::vec3(0.0f);
		if (KeyboardInput::Get().IsKeyPressed(EKey::W) || KeyboardInput::Get().IsKeyHeld(EKey::W)) {
			moveInput += forward;
		}
		if (KeyboardInput::Get().IsKeyPressed(EKey::S) || KeyboardInput::Get().IsKeyHeld(EKey::S)) {
			moveInput -= forward;
		}
		if (KeyboardInput::Get().IsKeyPressed(EKey::A) || KeyboardInput::Get().IsKeyHeld(EKey::A)) {
			moveInput -= right;
		}
		if (KeyboardInput::Get().IsKeyPressed(EKey::D) || KeyboardInput::Get().IsKeyHeld(EKey::D)) {
			moveInput += right;
		}

		// hit the ground (y = 0)
		playerMovement.isGrounded = transform.position.y <= 0.0f;
		if (playerMovement.isGrounded) {
			transform.position.y = 0.0f;
			playerMovement.velocity.t = 0.0f;
		}
		// jump
		if (playerMovement.isGrounded && KeyboardInput::Get().IsKeyPressed(EKey::Space)) {
			playerMovement.velocity.y = 5.0f;
			playerMovement.isGrounded = false;
		}

		// velocity control
		if (glm::length(moveInput))
			moveInput = glm::normalize(moveInput);

		playerMovement.velocity.x = moveInput.x * playerMovement.movementSpeed;
		playerMovement.velocity.z = moveInput.z * playerMovement.movementSpeed;

		if (!playerMovement.isGrounded) {
			playerMovement.velocity.y += playerMovement.gravity * dt;
		}

		transform.position += playerMovement.velocity * dt;
		camera.position = transform.position;

	}
};