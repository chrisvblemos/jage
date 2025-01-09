
#pragma once

#include <bitset>
#include <queue>
#include <array>
#include <cassert>

using Entity = uint32_t;
inline constexpr Entity MAX_ENTITIES = 5000;
using ComponentType = uint8_t;
inline constexpr ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

class EntityManager {
public:
	EntityManager() {
		for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
			mAvailableEntities.push(entity);
		}
	}

	Entity CreateEntity() {
		assert(mLivingEntityCount < MAX_ENTITIES && "EntityManager: Max limit reached for created entities.");

		Entity id = mAvailableEntities.front();
		mAvailableEntities.pop();
		++mLivingEntityCount;

		return id;
	}

	void DestroyEntity(Entity entity) {
		assert(entity < MAX_ENTITIES && "EntityManager: Entity out of range");

		mSignatures[entity].reset();
		mAvailableEntities.push(entity);
		--mLivingEntityCount;
	}

	void SetSignature(Entity entity, Signature signature) {
		assert(entity < MAX_ENTITIES && "EntityManager: Entity out of range");

		mSignatures[entity] = signature;
	}

	Signature GetSignature(Entity entity) {
		assert(entity < MAX_ENTITIES && "EntityManager: Entity out of range");
	
		return mSignatures[entity];
	}

private:
	std::queue<Entity> mAvailableEntities{};
	std::array<Signature, MAX_ENTITIES> mSignatures{};
	uint32_t mLivingEntityCount{};
};