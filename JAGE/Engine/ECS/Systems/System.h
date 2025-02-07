#pragma once

#include <Core/Core.h>

class System {
protected:
	std::set<Entity> entities{};

public:
	std::string name;
	bool entitiesChanged = false;

	virtual void Start() {};
	virtual void Update(float dt) = 0;
	virtual void PostUpdate() {};
	virtual void End() {};

	inline void Register(const Entity entity) {
		auto [it, res] = entities.insert(entity);
		entitiesChanged = res;
	}

	inline void UnRegister(const Entity entity) {
		size_t erased = entities.erase(entity);
		entitiesChanged = erased > 0;
	}

	inline const size_t GetEntityCount() const { return entities.size(); }
	inline const std::set<Entity>& GetEntities() const { return entities; }

};