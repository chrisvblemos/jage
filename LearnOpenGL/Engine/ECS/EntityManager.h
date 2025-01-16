
#pragma once

#include <Core/Core.h>

class EntityManager {
public:
	EntityManager() {
		for (Entity entity = 0; entity < Constants::MAX_ENTITIES; entity++) {
			mAvailableEntities.push(entity);
		}
	}

	Entity CreateEntity() {
		assert(mLivingEntityCount < Constants::MAX_ENTITIES && "EntityManager: Max limit reached for created entities.");

		Entity id = mAvailableEntities.front();
		mAvailableEntities.pop();
		++mLivingEntityCount;

		return id;
	}

	void DestroyEntity(Entity entity) {
		assert(entity < Constants::MAX_ENTITIES && "EntityManager: Entity out of range");

		mSignatures[entity].reset();
		mAvailableEntities.push(entity);
		--mLivingEntityCount;
	}

	void SetSignature(Entity entity, Signature signature) {
		assert(entity < Constants::MAX_ENTITIES && "EntityManager: Entity out of range");

		mSignatures[entity] = signature;
	}

	Signature GetSignature(Entity entity) {
		assert(entity < Constants::MAX_ENTITIES && "EntityManager: Entity out of range");
	
		return mSignatures[entity];
	}

private:
	std::queue<Entity> mAvailableEntities{};
	std::array<Signature, Constants::MAX_ENTITIES> mSignatures{};
	uint32_t mLivingEntityCount{};
};