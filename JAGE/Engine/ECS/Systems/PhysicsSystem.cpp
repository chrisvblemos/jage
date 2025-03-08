
#include <ECS/Components/Transform.h>
#include <ECS/Components/RigidBody.h>
#include <World/World.h>
#include <Physics/Physics.h>
#include "PhysicsSystem.h"

void PhysicsSystem::Update(float dt) {
	for (const Entity& entity : entities) {

		RigidBody& rigidBody = World::Get().GetComponent<RigidBody>(entity);
		Transform& transform = World::Get().GetComponent<Transform>(entity);

		Physics::AddRigidBody(entity, transform.position, transform.rotation);
		transform.position = Physics::GetRigidBodyPosition(entity);
		transform.rotation = Physics::GetRigidBodyRotation(entity);
		rigidBody.velocity = Physics::GetRigidBodyVelocity(entity);
	}

	const float fixedTimeStep = 1.0f / 60.0f;  // 60 updates per second
	static float accumulator = 0.0f;
	accumulator += dt;

	while (accumulator >= fixedTimeStep)
	{
		Physics::Update(fixedTimeStep);
		accumulator -= fixedTimeStep;
	}
}