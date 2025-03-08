#include <ECS/Components/Character.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>
#include <World/World.h>
#include "CharacterSystem.h"

void CharacterSystem::Update(float dt) {
	for (const Entity& entity : entities)
	{
		World& world = World::Get();
		Transform& transform = world.GetComponent<Transform>(entity);
		Character& character = world.GetComponent<Character>(entity);

		Camera& cam = world.GetComponent<Camera>(character.camera);
		Transform& camTransform = world.GetComponent<Transform>(character.camera);

		float mouseSens = 10 * cfg::Input.Read<float>("Mouse", "input.mouse.sensitivity", 1.0f);
		const Vec2 mouseDelta = mouseSens * Input::Get().GetMouseDelta();

		if (mouseDelta.x) {
			camTransform.RotateAround(WUp, -mouseDelta.x * dt);
			if (character.cameraControlsYaw)
				transform.RotateAround(WUp, -mouseDelta.x * dt);
		}

		if (mouseDelta.y) {
			Vec3 camRight = transform.rotation * WRight;
			camTransform.RotateAround(camRight, -mouseDelta.y * dt);
		}		

		Vec3 moveInput = Vec3(0.0f);
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_W) || Input::Get().GetKeyWasPressed(GLFW_KEY_W))
			moveInput += transform.forward;
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_S) || Input::Get().GetKeyWasPressed(GLFW_KEY_S))
			moveInput -= transform.forward;
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_A) || Input::Get().GetKeyWasPressed(GLFW_KEY_A))
			moveInput -= transform.right;
		if (Input::Get().GetKeyWasPressed(GLFW_KEY_D) || Input::Get().GetKeyWasPressed(GLFW_KEY_D))
			moveInput += transform.right;

		// hit the ground (y = 0)
		character.isGrounded = transform.position.y <= 0.0f;
		if (character.isGrounded) {
			transform.position.y = 0.0f;
			character.velocity.t = 0.0f;
		}
		if (character.isGrounded && Input::Get().GetKeyWasPressed(GLFW_KEY_SPACE)) {
			character.velocity.y = 5.0f;
			character.isGrounded = false;
		}

		// normalize speed
		if (glm::length(moveInput))
			moveInput = glm::normalize(moveInput);

		character.velocity.x = moveInput.x * character.maxMovementSpeed;
		character.velocity.z = moveInput.z * character.maxMovementSpeed;

		if (!character.isGrounded) {
			character.velocity.y += character.gravity * dt;
		}

		transform.position += character.velocity * dt;
		camTransform.position = transform.position;
	}
};