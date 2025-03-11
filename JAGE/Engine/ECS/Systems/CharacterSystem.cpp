#include <World/World.h>
#include "CharacterSystem.h"

void CharacterSystem::Update(float dt) {
	if (characterEntity == -1) {
		for (const Entity& entity : entities)
		{
			characterEntity = entity;
			break;
		}
	}
	else {
		World& world = World::Get();
		Transform& characterTransform = world.GetComponent<Transform>(characterEntity);
		Character& character = world.GetComponent<Character>(characterEntity);

		Camera& characterCamera = world.GetComponent<Camera>(character.camera);
		Transform& characterCameraTransform = world.GetComponent<Transform>(character.camera);

		float mouseSens = 10 * cfg::Input.Read<float>("Mouse", "input.mouse.sensitivity", 1.0f);
		const Vec2 mouseDelta = mouseSens * Input::Get().GetMouseDelta();

		if (mouseDelta.x) {
			characterCameraTransform.RotateAround(WUp, -mouseDelta.x * dt);
			if (character.cameraControlsYaw)
				characterTransform.RotateAround(WUp, -mouseDelta.x * dt);
		}

		if (mouseDelta.y) {
			Vec3 camRight = characterTransform.rotation * WRight;
			characterCameraTransform.RotateAround(camRight, -mouseDelta.y * dt);
		}

		if (Input::Get().GetKeyIsHeld(GLFW_KEY_W) || Input::Get().GetKeyIsHeld(GLFW_KEY_W))
			moveInput += characterTransform.forward;
		if (Input::Get().GetKeyIsHeld(GLFW_KEY_S) || Input::Get().GetKeyIsHeld(GLFW_KEY_S))
			moveInput -= characterTransform.forward;
		if (Input::Get().GetKeyIsHeld(GLFW_KEY_A) || Input::Get().GetKeyIsHeld(GLFW_KEY_A))
			moveInput -= characterTransform.right;
		if (Input::Get().GetKeyIsHeld(GLFW_KEY_D) || Input::Get().GetKeyIsHeld(GLFW_KEY_D))
			moveInput += characterTransform.right;

		character.wantsToJump = character.wantsToJump? character.wantsToJump : Input::Get().GetKeyWasPressed(GLFW_KEY_SPACE);
		characterCameraTransform.position = characterTransform.position;
	}
}
void CharacterSystem::FixedUpdate(float dt)
{
	if (characterEntity == -1) return;

	World& world = World::Get();
	Transform& characterTransform = world.GetComponent<Transform>(characterEntity);
	Character& character = world.GetComponent<Character>(characterEntity);

	// hit the ground (y = 0)
	character.isGrounded = characterTransform.position.y <= 0.0f;
	if (character.isGrounded) {
		characterTransform.position.y = 0.0f;
		character.velocity.t = 0.0f;
	}
	if (character.isGrounded && character.wantsToJump) {
		character.velocity.y = 5.0f;
		character.isGrounded = false;
		character.wantsToJump = false;
	}

	// normalize speed
	if (glm::length(moveInput) > 0.01)
		moveInput = glm::normalize(moveInput);

	character.velocity.x = moveInput.x * character.maxMovementSpeed;
	character.velocity.z = moveInput.z * character.maxMovementSpeed;

	if (!character.isGrounded) {
		character.velocity.y += character.gravity * dt;
	}

	characterTransform.position += character.velocity * dt;
	moveInput = Vec3(0.0f);
}
;