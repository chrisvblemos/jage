#include "ComponentManager.h"

void ComponentManager::EntityDestroyed(Entity entity) {
	for (auto const& pair : mComponentArrays) {
		auto const& component = pair.second;
		component->EntityDestroyed(entity);
	}
}