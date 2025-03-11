#include <ECS/Components/Transform.h>
#include <World/World.h>
#include "TransformSystem.h"

void TransformSystem::FixedUpdate(float dt)
{
	World& world = World::Get();
	for (const Entity& entity : entities) {
		Transform& transform = world.GetComponent<Transform>(entity);
		transform.forward = transform.rotation * -WForward;
		transform.up = transform.rotation * WUp;
		transform.right = transform.rotation * WRight;
	}
}
