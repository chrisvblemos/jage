
#pragma once

#include <Core/Core.h>

class EntityManager {
public:
	EntityManager() {
		for (Entity entity = 0; entity < Constants::MAX_ENTITIES; entity++) {
			mAvailableEntities.push(entity);
		}
	}

	const std::string& GetEntityName(const Entity entity) const {
		auto it = nameMap.find(entity);
		if (it != nameMap.end())
			return it->second;
		else
			return "<InvalidEntityName>";
	}

	const std::vector<Entity> GetActiveEntities() const {
		return std::vector<Entity>(activeEntities.begin(), activeEntities.end());
	}

	Entity CreateEntity(const std::string& name = "Entity") {
		assert(mLivingEntityCount < Constants::MAX_ENTITIES && "EntityManager: Max limit reached for created entities.");

		Entity entity = mAvailableEntities.front();
		mAvailableEntities.pop();
		++mLivingEntityCount;

		activeEntities.insert(entity);

		// assign unique name to game object when creating it
		nameMap[entity] = GenerateUniqueEntityName(name);

		return entity;
	}

	void DestroyEntity(const Entity entity) {
		assert(entity < Constants::MAX_ENTITIES && "EntityManager: Entity out of range");

		// handle freeing entity name
		const std::string& name = nameMap[entity];
		auto it = nameUsage.find(name);
		if (it != nameUsage.end() && --it->second == 0) {
			nameUsage.erase(it);
		}

		activeEntities.erase(entity);

		mSignatures[entity].reset();
		mAvailableEntities.push(entity);
		--mLivingEntityCount;
	}

	void SetSignature(const Entity entity, const Signature signature) {
		assert(entity < Constants::MAX_ENTITIES && "EntityManager: Entity out of range");

		mSignatures[entity] = signature;
	}

	Signature GetSignature(const Entity entity) const {
		assert(entity < Constants::MAX_ENTITIES && "EntityManager: Entity out of range");
	
		return mSignatures[entity];
	}

private:
	std::unordered_map<std::string, uint32_t> nameUsage;
	std::unordered_map<Entity, std::string> nameMap;

	std::set<Entity> activeEntities;
	std::queue<Entity> mAvailableEntities{};

	std::array<Signature, Constants::MAX_ENTITIES> mSignatures{};
	uint32_t mLivingEntityCount{};

	std::string GenerateUniqueEntityName(const std::string& baseName) {
		uint32_t& count = nameUsage[baseName];
		if (count == 0) {
			++count;
			return baseName;
		}

		return std::format("{}({})", baseName, count++);
	}
};