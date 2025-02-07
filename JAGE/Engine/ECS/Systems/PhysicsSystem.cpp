
#include <ECS/Components/Transform.h>
#include <ECS/Components/RigidBody.h>
#include <World/World.h>
#include "PhysicsSystem.h"

void PhysicsSystem::Update(float dt) {
	for (const Entity& entity : entities) {
		RigidBody& rigidBody = World::Get().GetComponent<RigidBody>(entity);
		Transform& transform = World::Get().GetComponent<Transform>(entity);

		transform.position += rigidBody.velocity * dt;

		if (rigidBody.isGravityEnabled)
			rigidBody.velocity.y += rigidBody.gravity * dt;
	}
}