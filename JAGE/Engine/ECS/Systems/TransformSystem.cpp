#include <ECS/Components/Transform.h>
#include <World/World.h>
#include "TransformSystem.h"

void TransformSystem::Update(float dt)
{
	World& world = World::Get();
	for (const Entity& entity : entities) {
		Transform& transform = world.GetComponent<Transform>(entity);

		glm::quat qrot = glm::quat(transform.rotation);
		transform.forward = qrot * (-WForward);
		transform.up = qrot * WUp;
		transform.right = qrot * WRight;
	}
}
