#include <ECS/Components/PlayerMovement.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/Transform.h>
#include <World/World.h>
#include <Core/Utils.h>

#include "PlayerSystem.h"

const float MAX_CAMERA_PITCH = glm::radians(89.0f);
const float MIN_CAMERA_PITCH = glm::radians(-89.0f);

void PlayerSystem::Update(float dt) {
	if (mEntities.empty()) {
		return;
	}

	for (const Entity& entity : mEntities)
	{
		Transform& transform = World::Get().GetComponent<Transform>(entity);
		Camera& camera = World::Get().GetComponent<Camera>(entity);
		PlayerMovement& playerMovement = World::Get().GetComponent<PlayerMovement>(entity);

		// camera control
		const glm::vec2 mouseDelta = Input::Get().GetMouseDelta();

		// the player entity only rotates in the yaw axis
		// camera pitch is stored inside the camera component
		if (mouseDelta.x) {
			camera.yaw -= camera.mMouseSensitivity * mouseDelta.x * dt;
		}

		// clamp pitch between -89.0f and 89.0f
		if (mouseDelta.y) {
			camera.pitch -= camera.mMouseSensitivity * mouseDelta.y * dt;
			camera.pitch = glm::clamp(camera.pitch, MIN_CAMERA_PITCH, MAX_CAMERA_PITCH);
		}

		transform.rotation = glm::quat(glm::vec3(playerMovement.pitch, camera.yaw, 0.0f));
		camera.rotation = glm::quat(glm::vec3(camera.pitch, camera.yaw, 0.0f));
		
		const glm::vec3 forward = transform.Forward();
		const glm::vec3 right = transform.Right();

		// player movement control
		glm::vec3 moveInput = glm::vec3(0.0f);
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_W) || Input::Get().GetKeyWasPressed(GLFW_KEY_W)) {
			moveInput += forward;
		}
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_S) || Input::Get().GetKeyWasPressed(GLFW_KEY_S)) {
			moveInput -= forward;
		}
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_A) || Input::Get().GetKeyWasPressed(GLFW_KEY_A)) {
			moveInput -= right;
		}
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_D) || Input::Get().GetKeyWasPressed(GLFW_KEY_D)) {
			moveInput += right;
		}

		// hit the ground (y = 0)
		playerMovement.isGrounded = transform.position.y <= 0.0f;
		if (playerMovement.isGrounded) {
			transform.position.y = 0.0f;
			playerMovement.velocity.t = 0.0f;
		}
		// jump
		if (playerMovement.isGrounded && Input::Get().GetKeyWasPressed(GLFW_KEY_SPACE)) {
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

		LOG_DISPLAY_KEYED(Logging::vec3Str(camera.position), "Camera.position");
		LOG_DISPLAY_KEYED(Logging::quatStr(camera.rotation), "Camera.rotation");
		LOG_DISPLAY_KEYED(Logging::quatStr(transform.rotation), "Player.rotation");
	}
};