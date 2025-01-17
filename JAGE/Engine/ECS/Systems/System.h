#pragma once

#include <Core/Core.h>

class System {
protected:
	std::set<Entity> mEntities{};

public:
	std::string name;
	bool entitiesChanged = false;

	inline void Register(const Entity entity) {
		auto [it, res] = mEntities.insert(entity);
		entitiesChanged = res;
	}

	inline void UnRegister(const Entity entity) {
		size_t erased = mEntities.erase(entity);
		entitiesChanged = erased > 0;
	}

	inline const size_t GetEntityCount() const { return mEntities.size(); }
	inline const std::set<Entity>& GetEntities() const { return mEntities; }

};