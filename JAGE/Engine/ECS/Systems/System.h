#pragma once

#include <Core/Core.h>

class System {
protected:
	std::set<Entity> entities{};

public:
	std::string name;
	bool entitiesChanged = false;

	virtual void PreStart() {};
	virtual void Start() {};
	virtual void PostStart() {};

	void Step(float dt) {
		static float accumulator;
		const float fixedDeltaTime = 1 / 60.0f;

		accumulator += dt;
		while (accumulator >= fixedDeltaTime) {
			FixedUpdate(fixedDeltaTime);
			accumulator -= fixedDeltaTime;
		}

		Update(dt);
		PostUpdate();
	}

	virtual void Update(float dt) {};

	/* Physics time fixed step. i.e. dt is of fixed value, not frame dependent. */
	virtual void FixedUpdate(float dt) {};

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