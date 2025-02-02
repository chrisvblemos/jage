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

const Signature World::GetEntitySignature(const Entity entity) const{
	return mEntityManager->GetSignature(entity);
}

void World::DestroyEntity(const Entity entity)
{
	mEntityManager->DestroyEntity(entity);
	mComponentManager->EntityDestroyed(entity);
	mSystemManager->EntityDestroyed(entity);
}

const std::string World::GetEntityName(const Entity entity) const {
	return mEntityManager->GetEntityName(entity);
}

const std::vector<Entity> World::GetEntities() const
{
	return mEntityManager->GetActiveEntities();
}
