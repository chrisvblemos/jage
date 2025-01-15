#include "World.h"

bool World::Initialize()
{
	mComponentManager = std::make_unique<ComponentManager>();
	mEntityManager = std::make_unique<EntityManager>();
	mSystemManager = std::make_unique<SystemManager>();

	return true;
}

Entity World::CreateEntity()
{
	return mEntityManager->CreateEntity();
}

const Signature& World::GetEntitySignature(Entity entity) const{
	return mEntityManager->GetSignature(entity);
}

void World::DestroyEntity(Entity entity)
{
	mEntityManager->DestroyEntity(entity);
	mComponentManager->EntityDestroyed(entity);
	mSystemManager->EntityDestroyed(entity);
}